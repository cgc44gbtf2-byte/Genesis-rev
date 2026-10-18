# Managed class clans

All ten classes share `clan` governance: Vampire, Werewolf, Sorcerer, Drow,
Highlander, Mage, Dragon, Ninja, Monk and Elemental. Hybrids remain disabled.
Leader, Officer and Member are **administrative offices**, not combat ranks.
These commands never change generation, Werewolf rank, Dragon age, class
skills, disciplines, favored spheres, HP or mana. Existing class-title
wording and Covenants are unchanged; Covenants remain a separate system.

## Installation and adoption

Before upgrading, back up existing player files and runtime state. Install
the revised service unit and create `/var/lib/anathema/clans`, owned by
`anathema:anathema`, mode `0700`. It must not be a symlink. Follow INSTALL.md
for the protected parent and other writable directories. In a development
layout the path is `../clans` relative to the running area's directory.

On a fresh empty clan directory, startup writes a versioned empty registry.
No historical accounts are scanned or automatically granted authority.
Staff deliberately create/adopt each clan with its chosen **online,
awake, out-of-combat, classed player**:

```
clanadmin create Alice Order of Dawn
clanadmin list
```

If Alice already has a legacy clan label, the supplied label must match it.
Names contain 2-25 ASCII letters/digits/spaces/hyphens/apostrophes, with no
leading, trailing or repeated spaces. Existing colored/nonconforming labels
need deliberate normalization with `clanname` **before** adoption; this is
not an automatic bloodline or tradition migration.

Creation registers only the selected leader. Bring existing members into
the registry with invitations and individual acceptance; the roster includes
**registered members only**, not undiscovered legacy player files. Legacy
generation 1/2 titles do not automatically confer managed authority.
An unregistered legacy member may accept an invitation to the same named
class clan; membership in a differently named legacy clan must first be
resolved by staff. No discipline/tribe/sphere data are inferred from the label.

After adoption use managed commands, not `clanname`, to change membership.
The old native command names now delegate to this authorization system:

| Class | Existing aliases |
| --- | --- |
| Ninja | `humble` invites; `promote` appoints Officer; `demote` removes office; `outcast` removes membership |
| Drow | `dinduct` invites; `promotion` appoints Officer; `failure` removes office |
| Monk | `decide` invites; `expel` removes membership |
| Vampire / Werewolf | `favour <name> induct` invites; `outcast` removes; `elder` appoints Officer |
| Every class | `outcast <name>` delegates to `clan kick <name>` |

This deliberately replaces the unsafe old generation-changing management
logic. `favour elder` no longer toggles combat generation/rank or pretends to
grant Werewolf Elder rank; use `clan demote` to remove administrative office.
The previously advertised but unimplemented `favour sire` is not offered.
Legacy aliases require staff adoption first, and recruitment always waits
for acceptance regardless of blanket `vampire` consent flags.

## Player commands

```
clan info
clan roster [page]
clan invite Bob
clan accept Alice
clan decline
clan promote Bob
clan demote Bob
clan kick Bob
clan transfer Bob
clan leave
clan disband confirm
```

Rosters show 20 registered names/roles per page, including offline members.
Leaders appoint/remove officers and manage lower-ranked members. Officers
invite and remove ordinary members, but cannot promote, demote, remove peers
or remove the leader. Nobody promotes/removes themselves with those commands.
Promotions, demotions and kicks accept offline **registered** player names.
One player can have only one active managed clan, and each clan has exactly
one leader. Same clan labels in different classes are distinct organizations.

Invitations and transfers require both players to be online, standing,
out of combat, visible to one another, in the same room and plane, and the
same supported class. The recipient must type `clan accept <inviter>` within
120 seconds. Authority, membership, class and location are checked again
on acceptance. Offers expire, can be declined, and are discarded on restart
or copyover. Dominate/command/order/force/scripts cannot supply these commands
instead of a player's own socket input, including abbreviated aliases.

Only the current leader can offer leadership to an existing member. Their
named acceptance changes both offices in **one registry transaction**:
recipient becomes Leader and former leader becomes Officer. A leader cannot
leave without transferring or explicitly disbanding. Staff recovery can use
`clanadmin disband <id> confirm`; it removes all registered members, including
offline ones. No gameplay currency is charged for governance operations.

## Persistence and recovery

`clans/Registry` is the authority, not the `Clan` string cached in a player
file. Each accepted change validates the entire proposed registry, writes a
private temporary file, flushes/fsyncs it, atomically renames it, and fsyncs
the directory. A nonblocking process lock prevents two servers from writing
the same registry. Registered player labels are reconciled after an accepted
change and when a full player file loads, including copyover/reconnect loads.
No multi-player player-file transaction or save-format change is required.

Removed members retain registry tombstones, so an old offline player save
cannot restore their membership. Active managed identities cannot be
renamed, deleted, or class-cleared until their membership is released.
Registered names whose player files are missing cannot be reclaimed through
new-account creation. Ordinary generation editing does not grant clan office;
the administrative `generation` command now rejects NPCs, overflow and
values outside 1-13 (the existing pset range).

A pre-rename write failure leaves both the old registry and live membership
unchanged. A directory-fsync failure **after rename** retains the complete
visible new state, reports uncertain durability, and freezes further
governance writes, identity reuse and class resets. Staff use:

```
clanadmin retry
```

This retries the complete current state; only a durable success unfreezes
it. Do not roll back one participant's player file or infer success from
the cached display label. An invalid, unsafe, locked or unreadable registry
disables governance rather than treating the file as empty. A missing
registry beside an existing lock file also requires staff investigation.
Malformed data cannot be repaired with `retry`: stop the server, preserve
the evidence, restore a reviewed complete registry backup with correct
ownership/mode, and restart. A missing whole directory is indistinguishable
from an unconfigured development runtime; the installed systemd unit
requires its configured writable directory to exist.

Limits are 64 active clans, 1024 remembered player identities (including
removal tombstones), and 256 simultaneous offers. Capacity failures do not
charge resources or partially apply changes. Plan a versioned migration
before exhausting these limits; do not hand-delete tombstones. Back up the
**whole registry** as well as player/world data. There is no automated registry
backup, bulk legacy-account import, or production power-cut/sandbox test.
