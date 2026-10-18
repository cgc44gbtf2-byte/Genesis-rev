# Manual shutdown and copyover safety

## Upgrade and prerequisites

The copyover manifest is now versioned (`ANATHEMA_COPYOVER 1`), not compatible
with the previous unchecked format. **The first upgrade to this version must
be a normal cold restart**, not a copyover initiated by an older running binary.
Once this version is running, copyover to a compatible binary is supported.
Player-file grammar and class/power costs are unchanged.

The executable remains `../src/merc` relative to the area working directory,
including the installed service's root-owned `src` symlink. The handoff lives
at `copyover.data` in that directory. Its atomic directory must be owned by the
server UID and not group/other writable; ancestors must be trusted.

## Manual restart checkpoint

`shutdown` and `copyover` no longer force players to execute unchecked `save`
or resource-changing `call all` commands. Before proceeding they require:

- Every loaded real player is save-eligible, including switched originals and
  link-dead players. Initial sleepers must finish becoming eligible or quit
  under the existing no-save sleeper policy first.
- Every active game descriptor refers to an actual loaded original player.
- No active note/world editor, string editor, retained note draft or
  `AREA_CHANGED` area remains. Finish/discard drafts and save area changes first.
  `AREA_ADDED` is informational and does not permanently block saved areas.
- Every loaded real player's authoritative primary checkpoint succeeds with
  full atomic durability. Backup-only or uncertain directory-sync outcomes
  cannot authorize a restart. All player primary/backup destinations are
  attempted; a failed backup does not invalidate a durable primary.

Failure cancels the manual action without setting `merc_down`, writing a
shutdown marker, closing sessions, returning switched control or deleting NPCs.
Already completed individual snapshots are not rolled back. Automatic retry
metadata continues under the autosave policy while players remain loaded.

The dirty-area guard relies on existing OLC dirty tracking. This is not an
atomic transaction for area saves, clan rosters, notes and all world state.
Operators must verify their OLC saves separately.

## Copyover handoff

The dedicated `restart.c` module replaces the legacy unbounded parser and
unchecked manifest I/O. A successful checkpoint is followed by:

1. Validation of the listening socket/port, executable access, client sockets,
   unique descriptors/player names and bounded host/name tokens.
2. A private 0600 atomic manifest with checked writes, flush, file sync, close,
   rename and directory sync. Player-name validation remains ASCII alphabetic;
   the manifest writer only accepts its one fixed basename.
3. Saved socket flags followed by temporary inheritance changes. Listener and
   playing clients survive exec. Partial-login sockets and the reserve handle
   close **only if exec succeeds**, using `FD_CLOEXEC`.
4. `execl` of the configured executable, preserving the PID and listener.

On an exec/flag/manifest failure, original descriptor flags are restored,
the newly published manifest is removed, and all original session/control/
reserve handles remain in use. Diagnostics identify manifest or exec errors
without dumping player data. No failed attempt forces familiar recall.

The new process validates exact argument count, decimal range, recovery marker,
listener socket and port before boot/recovery. It reads only a private regular
non-symlink manifest, with complete bounded validation before adopting any
player: version, newline/sentinel/EOF, numeric ranges, connected sockets,
duplicate names/FDs and valid player tokens. FIFOs cannot block the reader.
Malformed recovery returns failure to main, which exits nonzero and closes its
inherited handles on process exit instead of looping or reporting normal stop.

Successful recovery reloads the saved original players in playing state.
Switched users return to their original player bodies; NPCs, pets, combat
relationships, world contents, drafts and other live process memory are not
serialized as a continuing world. Partial logins must reconnect. A missing or
invalid individual player file closes that connection rather than fabricating
a replacement character.

## Verification

The 26th full-engine suite covers failed-primary shutdown refusal, eligibility/
editor/dirty-area guards, active descriptor membership, six manifest I/O
failure stages, exec failure and exact socket/reserve-flag restoration,
argument validation, malformed/truncated/duplicate/oversized-number records,
symlink/FIFO refusal and successful native player recovery.

`tests/smoke_copyover.py` boots the original world in a private temporary
runtime with one synthetic staff account, no historical accounts or notes.
It injects a missing primary directory to prove shutdown and copyover refuse,
then performs a real production `execl`, retains the original PID/active TCP
session, verifies a pending-login socket closes and the inherited listener
accepts another connection, saves again and performs checked shutdown.
The existing greeting/TERM smoke remains independent.

## Limits

An executable may pass access checks but fail during dynamic loading or boot
**after successful exec**. The old process cannot be resumed then; the durable
primary checkpoints let users reconnect after a cold restart. This is not
world-state journaling or a rollback-capable deployment.

Retries remain RAM-only. Backups can remain stale if their checkpoint fails.
Permanent storage errors, crashes/power loss before durability, late-quit
checkpoint fallback, direct staff extraction, body-part provenance and
class-reset gear/stat reconciliation remain as documented elsewhere.

External TERM/systemd stop is deliberately different: it attempts final saves
and warns on failure, but cannot refuse a service manager's stop indefinitely.
No production deployment, live systemd sandbox run, power-cut test or long
multiplayer soak is implied by the isolated checks.
