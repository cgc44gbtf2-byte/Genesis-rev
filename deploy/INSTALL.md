# Debian / Ubuntu systemd installation

This is an operator-run installation, not an installer script. Review each
step before executing it on the target host. Nothing here requires the game
to run as root. Examples use TCP 4000 on Debian 12/13 or Ubuntu 22.04/24.04
with systemd. Resolve the original Diku/Merc/God Wars licensing and permission
requirements before operating or redistributing the game; this deployment
does not grant them.

## 1. Prepare and preserve existing state

**Do not copy these commands over an existing installation blindly.** Inspect
`/opt/anathema`, `/var/lib/anathema`, the `anathema` user/group and any existing
unit first. If they exist, follow the upgrade section instead of the fresh
directory-creation and import steps.

For an existing server, announce maintenance, save players and OLC changes
in-game, then stop its supervisor/service and verify the process has exited.
Only then take a consistent backup of the **entire** runtime tree, including
area files, player files, backups, notes, logs, ban/disabled lists and other
local state. Save the current executable, unit, configuration and ownership
information too. Keep backups outside service-writable directories, private,
off-host where possible, and test restoration.

For example, after stopping an existing installation with the layout below,
use a **new, unique** filename (replace the timestamp):

```sh
sudo tar --acls --xattrs --numeric-owner -C /var/lib \
  -czf /root/anathema-state-YYYYMMDD-HHMMSS.tar.gz anathema
sudo chmod 0600 /root/anathema-state-YYYYMMDD-HHMMSS.tar.gz
```

Never import the historical archive's `player/`, `backup/`, notes, logs,
accounts, credentials, bans, disabled commands or leaderboards. Existing
operator-owned live state is a separate, deliberate migration, not an
archive import. Player files and backups contain credential hashes and
private information; do not commit, publish or serve them over HTTP.

## 2. Build as an ordinary user

From the patched repository root (the directory containing `Genesis/src`):

```sh
sudo apt-get update
sudo apt-get install --no-install-recommends build-essential libcrypt-dev \
  git ca-certificates unzip
make -j2
make test
```

Build `Genesis/src`, **not** source/object files or the executable in
`Genesis.zip`. Use a normal, non-sanitized build for the service. Never enable
`NOCRYPT`; libcrypt/yescrypt support is required. Build/test without root.
If running `make sanitize` separately, rebuild the normal executable afterward.

## 3. Fresh account and filesystem layout

The following is for a **new** installation with no existing user, group or
target directories. If any target exists, stop and inspect it; do not remove
or overwrite it to make these commands succeed.

```sh
sudo adduser --system --group --home /var/lib/anathema \
  --no-create-home --shell /usr/sbin/nologin anathema
sudo install -d -o root -g root -m 0755 /opt/anathema \
  /opt/anathema/src /opt/anathema/deploy /var/lib/anathema
sudo install -d -o anathema -g anathema -m 0700 \
  /var/lib/anathema/area /var/lib/anathema/player \
  /var/lib/anathema/backup /var/lib/anathema/notes /var/lib/anathema/log \
  /var/lib/anathema/clans
sudo install -o root -g root -m 0755 Genesis/src/merc /opt/anathema/src/merc
sudo install -o root -g root -m 0644 deploy/INSTALL.md /opt/anathema/deploy/INSTALL.md
sudo ln -s /opt/anathema/src /var/lib/anathema/src
```

The service must **not** own `/var/lib/anathema` itself, `/opt/anathema`, the
executable or the symlink. Do not recursively chown `/var/lib/anathema`.
Only the six runtime subdirectories above are service-writable. The
root-owned parent prevents replacing `src` with a malicious copyover target;
`ProtectSystem=strict` also makes the executable and symlink read-only inside
the service. Keep all ancestors root-owned and non-group/world-writable.
The symlink command deliberately has no `-f`: inspect any existing path
instead of replacing it.

The exact relative paths matter:

| Source behavior | Installed path |
| --- | --- |
| Current working directory; `area.lst` and its listed files | `/var/lib/anathema/area` |
| `PLAYER_DIR` = `../player/` | `/var/lib/anathema/player` |
| `BACKUP_DIR` = `../backup/` | `/var/lib/anathema/backup` |
| Managed clan registry and process lock | `/var/lib/anathema/clans` |
| Board notes and their archives | `/var/lib/anathema/notes` |
| Legacy file logs, if used | `/var/lib/anathema/log` |
| OLC area saves, `area.lst`, `ban.txt`, `disabled.txt`, `leader.txt`, `shutdown.txt` | writable `area/` |
| `COPYOVER_FILE` = `copyover.data` | `/var/lib/anathema/area/copyover.data` |
| `EXE_FILE` = `../src/merc` | `/var/lib/anathema/src/merc` via the protected symlink |

`comm.c` starts in the foreground; it does not daemonize or change directory.
It takes the port as its first argument (default 4000, must be above 1024).
No PID file, wrapper loop or socket unit is needed.

## 4. Install reviewed world data, not historical accounts

The source checkout does not include the runtime world. Supply authorized,
reviewed `.are` / `.hlp` files and `area.lst`. If using the retained archive,
first list its contents, then extract **only** these world-data names into
a new, private staging directory as your ordinary user:

```sh
unzip -l Genesis.zip
umask 077
stage=$(mktemp -d)
unzip Genesis.zip 'Genesis/area/*.are' 'Genesis/area/*.hlp' \
  'Genesis/area/area.lst' -d "$stage"
```

Review the staged files before installing them. Inspect `area.lst` and ensure
every referenced file exists, stays within the area directory and is intended
for this world. Do not import absolute paths, traversal paths, symlinks, stale
`copyover.data` or other historical runtime files. Review world/help text for
private or inappropriate historical material too. Do not extract the whole
archive into the checkout or runtime directories.

For the fresh, empty `area/` directory only, after review:

```sh
sudo install -o anathema -g anathema -m 0600 \
  "$stage"/Genesis/area/*.are "$stage"/Genesis/area/*.hlp \
  "$stage"/Genesis/area/area.lst /var/lib/anathema/area/
```

If sourcing world data elsewhere, substitute that reviewed staging directory.
Leave `player`, `backup`, `notes` and `log` empty on a fresh installation.
Do not populate any historical privileged account to gain administrator
access; administrator provisioning is a separate, audited operator task.

## 5. Firewall first, then install and start the unit

**The game binds IPv4 INADDR_ANY, not just localhost. Telnet traffic, including
passwords, is plaintext.** Password hashing at rest does not protect it on the
wire. Establish a host and provider firewall **before** starting the server.
For private use, keep TCP 4000 blocked externally and use a VPN or SSH tunnel.
Do not assume a reverse proxy makes direct port 4000 safe.

On hosts already using UFW, inspect the policy with `sudo ufw status verbose`.
With an established default-deny incoming policy, an optional restricted rule
is `sudo ufw allow from 192.0.2.10 to any port 4000 proto tcp` (replace the
documentation address with the actual trusted client/VPN address).
Do not enable/reset a firewall remotely without first preserving SSH access.
If intentionally offering public plaintext telnet, explicitly accept that
risk before adding a public TCP 4000 rule; tell users never to reuse passwords.
Check both host/provider policy and IPv4/IPv6 exposure.

Review the unit, then install it:

```sh
sudo install -o root -g root -m 0644 deploy/anathema.service \
  /etc/systemd/system/anathema.service
sudo systemd-analyze verify /etc/systemd/system/anathema.service
sudo systemctl daemon-reload
sudo systemctl start anathema.service
sudo systemctl status anathema.service
sudo journalctl -u anathema.service -n 100 --no-pager
```

Enable boot startup only after checking a successful boot, restricted
connectivity, a newly created test character's save/reload, and OLC writes:

```sh
sudo systemctl enable anathema.service
```

For another unprivileged port, use `sudo systemctl edit anathema.service`:

```ini
[Service]
ExecStart=
ExecStart=/opt/anathema/src/merc 5000
```

Update firewall rules accordingly. Changes require a stop/start, not a reload.
`daemon-reload` only reloads systemd's configuration; it does not reload the
game. Do not add `ExecReload` sending HUP: HUP is not a supported save/reload
interface and can terminate the process without saving.

## 6. Saves, stops, copyover and upgrades

* TERM/INT request shutdown through a signal flag; eligible loaded players
  are saved in normal-loop context before normal exit. This requires the
  patched graceful-shutdown implementation, not the archive executable.
  HUP and fatal signals are **not** graceful-save interfaces.
* `systemctl stop` sends TERM. `TimeoutStopSec=60s` bounds the wait; a stuck
  loop, filesystem stall or failed write can prevent completion and systemd
  may then SIGKILL. This is not a durability guarantee. Never use SIGKILL as
  the routine shutdown method. Announce maintenance and save in-game first.
* Save eligibility is unchanged: NPCs and players below level 2 are not saved
  by `save_char_obj`. Normal saves retain connection-state restrictions;
  graceful shutdown also saves fully loaded players in editors and notes.
  Player and backup files are each written to a private temporary file,
  checked and synced, then atomically renamed and the directory synced.
  Failures before rename preserve the previous file. A directory-sync
  failure after rename leaves the new file visible but reports uncertain
  durability. Primary and backup writes are not one transaction.
  Disk-full, permission errors, forced kills and power loss can still prevent
  recent changes from being persisted.
  Runtime `backup/` files are not a substitute for independent backups.
* Save OLC changes explicitly with the in-game `asave` facilities before
  maintenance. Copyover's area-autosave call is commented out in `act_wiz.c`;
  neither copyover nor service restart guarantees preservation of unsaved
  world edits. Player saves do not serialize the entire live world.
* The in-game privileged `copyover` command writes `area/copyover.data`,
  saves eligible players and execs `../src/merc` with the listening socket
  and connected descriptors inherited. The PID remains under systemd.
  Some login sessions are dropped. This unit deliberately allows exec and
  networking, does not use socket activation, and leaves area data writable.
  Copyover is not a systemd restart or a guaranteed recovery mechanism.
  Fatal signals use normal OS termination rather than attempting to save
  corrupted state or perform signal-handler copyover.
* `systemctl restart` disconnects everyone and performs a cold boot. It
  cannot restore sockets from a previous process's `copyover.data`.
  Never manually pass `copyover` arguments or reuse stale descriptor records
  after a cold start. The unit always starts with only the port argument.
* `Restart=on-failure` retries failures after 10 seconds, limited to three
  starts per five minutes. Clean exits (including in-game shutdown) remain
  stopped; explicit `systemctl stop` is not restarted. If the limit trips,
  diagnose the failure before `sudo systemctl reset-failed anathema.service`
  and a deliberate start. There is no application readiness notification.

For upgrades: save, stop, verify exit, and make the private backups described
above. Build/test as an ordinary user. Stage the new executable root-owned on
the same filesystem, then replace the stopped executable by rename (rather
than truncating a running binary). Preserve the data tree, permissions and
protected symlink. Review and verify unit changes, daemon-reload if needed,
then start and check the journal and save/reload behavior. Do not perform
unattended live binary replacement or assume cross-version copyover works.
After password migration, rollback to historical/non-yescrypt binaries is
not supported; restore only a deliberately chosen compatible build/state pair.

The sandbox reduces host access but does not make this legacy C game safe to
expose publicly. It retains network/DNS access and write access to game state.
Do not add blanket `ReadOnlyPaths` for area, `PrivateNetwork`, socket-binding
denials, `NoExecPaths=/`, or arbitrary syscall filters without testing boot,
save, OLC and descriptor-preserving exec. `LimitNOFILE=1024` matches the
select-based design; raising it is not a supported scaling fix. Journal
output can contain player identifiers; restrict log and backup access.
