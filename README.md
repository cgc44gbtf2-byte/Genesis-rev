# Godwars: Anathema

Class gear inventory, prices, corrected bugs and remaining content gaps:
[deploy/CLASS_GEAR.md](deploy/CLASS_GEAR.md).

Drow can craft supplies with `drowarmor bolts [1-20]` at 100 Drow points per
bolt (default one, no quest/gold/primal charge); firing still costs 500 points.

Conjured-supply resale and timed-item shop safeguards are documented in
[the consumable economy audit](deploy/CLASS_GEAR.md#conjured-consumables-and-market-follow-up).

Managed class-clan governance is documented in
[deploy/CLAN_MANAGEMENT.md](deploy/CLAN_MANAGEMENT.md). Staff adoption is
required before legacy clan-management aliases confer authority. Offices are
independent of class power ranks; existing titles and Covenants are unchanged.

Reliability and modernization branch of Genesis (God Wars MUD).
The Git branch is `Godwars-Anathema`: Git does not allow spaces or colons
in branch names.

## Source and historical archive

**Build and edit `Genesis/src/`.** The original `Genesis.zip` is retained
unchanged as a historical snapshot, not the patched distribution. Do not
extract its old source or object files over this tree.

Only source files were unpacked into version control. Historical player
backups, notes, logs, executables and object files were not republished as
individual files. The archive still contains historical runtime data:
do not use its accounts or credentials on a public server.

All original copyright notices are preserved. The source refers to Diku,
Merc and God Wars licensing/permission requirements; the referenced license
documents were not present in the supplied archive. This branch does not
grant new permissions. Resolve those requirements before operating or
redistributing the game.

## Build

Requires a C17-capable GCC-compatible compiler, GNU Make, and libcrypt
development files (for example `build-essential libcrypt-dev` on Ubuntu).

Debian 12/13 and Ubuntu 22.04/24.04 are covered by the CI container matrix.
On those systems:

```sh
sudo apt-get update
sudo apt-get install -y build-essential libcrypt-dev git ca-certificates
```

Use GCC for the regression suite (one isolated test uses a GCC sanitizer
parameter to discard an unrelated command table). No administrator/root
privileges are needed to build or run the MUD on an unprivileged port.

```sh
make -j2
make test
make sanitize
make -j2
```

The executable is `Genesis/src/merc`. `make sanitize` cleans the build and
runs the isolated regression suites under AddressSanitizer and
UndefinedBehaviorSanitizer; rebuild afterwards for normal use.
`LDLIBS` may be overridden for a platform-specific libcrypt path.
Never enable `NOCRYPT` to work around a missing library.

The historical Memwatch allocator is optional: `make clean` followed by
`make MEMWATCH=1`. Do not combine it with sanitizer tests.

For an isolated local runtime, supply the original area `.are` and `.hlp`
files plus `area.lst` under `Genesis/area/`, and create empty sibling
`player`, `backup`, `notes`, and `log` directories. Run `../src/merc 4000`
from `Genesis/area`. Do not import historical accounts. The server listens
on all interfaces by default; restrict access with the host firewall.

## Verified fixes

- GNU17 build, correct library link order, automatic header dependencies,
  and single definitions for leaderboard and extra-description globals.
- Removed alternate/master login credential acceptance and the embedded
  credential. Shared password verification rejects pass-the-hash login and
  handles crypt failures, including account deletion.
- Descriptor reads handle capacity, interruption and would-block correctly.
  Writes preserve pending bytes under backpressure and reject zero progress.
- Output overflow no longer frees a descriptor underneath its caller.
  Trailing color markers respect explicit input lengths; buffers terminate.
  Retried output does not regenerate prompts.
- Empty-string checks inspect string contents rather than pointer addresses.
- `channels +all/-all` no longer applies an uninitialized bit mask.
- Geas uses the charm skill identifier rather than an uninitialized value.
- Friction Curse parses quintessence correctly, initializes and retains
  adjusted spell power, checks mana before consuming quintessence, and
  restores the caster's room after a safe-room refusal.
- Unknown dragon skills return explicitly.
- Command parsing is sequenced and bounded. Removed broken allocation
  diagnostics that read uninitialized locals and could access a character
  after a command freed it.
- Data readers distinguish EOF from byte 255, terminate final unquoted words
  at EOF, reject incomplete quotes/strings, and bound string-pool writes.
  Error reporting no longer hangs on an EOF without a trailing newline.
- Corrected Memwatch neighbor integrity checks and bounded logging copies.

## Verification and remaining scope

Seventeen regression suites cover network/authentication, data readers,
editor/command buffer boundaries, atomic player saves and signal handling,
and gameplay fixes. Normal and ASan/UBSan suites passed locally. The complete
server compiled and booted with original area/help data in an isolated
runtime, and returned a TCP greeting. No live player data was loaded.

This is not a claim that the entire legacy game is secure or bug-free.
Compiler warnings still identify legacy formatting/size risks and unused
variables. Other
gameplay paths, non-player persistence, privileged commands and
long-running multiplayer behavior require further work. Some unflagged
legacy plaintext passwords resembling encoded hashes are deliberately
rejected; affected accounts require an operator-assisted reset.

## Password storage and second hardening pass

New and changed passwords use libxcrypt yescrypt with OS-generated random
salts (cost 5, approximately 16 MiB). Missing entropy, unsupported hashing,
or hashing failures reject the operation rather than falling back to weak
hashes. `NOCRYPT` builds are deliberately rejected.

Existing DES and eligible legacy plaintext credentials still verify and
are upgraded after successful authentication. The fully loaded character
is saved through the checked player-save path once playing, including
level-2 accounts and reconnect/takeover paths. The short login record is
never saved over the full inventory. A failed write does not clear the
pending migration flag. Modern hashes are not regenerated on every login.

Back up player files before upgrading. After migration, use only this
yescrypt-capable build; historical binaries are not a supported rollback.
Legacy DES verifies only its historical password prefix; users should
change their password after migration. Ordinary autosave eligibility
remains unchanged; migration uses player-save eligibility. Player-file writes use the atomic save
path described below, but disk failures can still prevent a migration from
being persisted.

The second pass also bounds capitalization and command tokens, repairs
editor empty-string/overflow cases, prevents shared-string mutation and
replacement leaks, terminates end-of-line reads at EOF, and replaces the
bug logger's unsafe arbitrary printf interpretation with bounded formatting.

## Atomic saves and systemd

Primary and backup player saves now use same-directory exclusive temporary
files with mode 0600, checked serialization/flush/file sync/close, atomic
rename, then directory sync. Pre-rename failures preserve the previous
save. A directory-sync failure after rename is reported as uncertain
durability: the new file is visible, but survival across power loss is not
guaranteed. Abrupt death can leave `.save-*` temporary files; they are not
loaded as player names. Backups and primary saves are individually atomic,
not a single transaction.

Player and backup directories must be owned by the service user and must
not be group/world writable. Symlink directories/targets are rejected;
keep ancestor directories trusted too. This prevents unsafe deployments
from silently weakening file protection. Inspect permissions when migrating
an older installation.

SIGTERM/SIGINT now request shutdown in the main loop, which saves fully
loaded players and closes connections before a normal exit. Fatal signals
do not attempt unsafe emergency saves. A forced kill or power failure can
still lose changes since the most recent successful save.

See [Debian/Ubuntu installation](deploy/INSTALL.md) and the reviewed
[systemd unit](deploy/anathema.service). They use a dedicated unprivileged
account, a root-owned read-only executable and restricted writable data
directories. No service is installed or enabled by this repository.

CI validates the unit without starting systemd, runs seventeen normal/sanitizer
suites, and boots an isolated world then verifies a clean SIGTERM exit.
Unit validation does not prove the sandbox or service operates correctly
on every host; follow the installation guide's host-level checks.

## Class-selection and power audit

Class acquisition tests exercise all ten classes, raw-stat thresholds,
supported vampire/mage/dragon specializations and owned clan strings.
Malkavian was advertised without any implemented discipline mapping; it is
now omitted rather than assigned invented powers.

The subsequent all-class pass repairs chained attacks after target extraction,
Icicle Storm's missing mana payment, Drow crossbow room-context cleanup,
named targeting and Vampire power-list names/ranks. It also hardens object-form
recovery when the bundled world's Hell room is absent. See
[the audit and limitations](deploy/CLASS_AUDIT.md).
Control Weather and Energy Drain now have
[guarded EM adaptations](deploy/RESTORED_SPELLS.md), with actual-engine
regressions and explicit differences from EM's damage/healing rules.

Power regressions cover Ninja bomb safety, Monk group-blessing initialization,
Drow flight toggling, Elementalist flying-target immunity and independent
per-target area damage, Sorcerer area-damage isolation, and Werewolf granite
walls on one-way, blocked and mismatched exits. Mage spells reject malformed,
negative or excessive quintessence, restore the caster after remote safety
checks, and validate covered deterministic targeting failures before payment.
Inferno initializes its botch state for nonpositive spell power.

This is a targeted correctness pass, not a full balance or ability audit.
Other explicitly disabled powers and ambiguous design choices remain unchanged;
Stun Fist was subsequently restored with the safeguards described below.

See [class catalog, costs, raw-stat caps and known gaps](deploy/CLASS_AUDIT.md)
for the follow-up audit. The added Python catalog check detects duplicate
command registrations, missing command implementations and unresolved literal
skill references; run `python3 tests/test_catalog.py` alongside `make test`.
Follow-up regressions cover Healing Touch affordability, Ninja Vanish Ki,
Werewolf ward dispatch, Mage listing thresholds and class stat caps/costs.

### Guarded Stun Fist restoration

Monk Technique 7 now enables `stunfist [target]` for 5,000 mana: about one
second of action/attack interruption, two seconds of attacker recovery, and
six seconds of per-target Stun Fist immunity measured from application.
Invalid, protected and immune targets cost nothing. Explicit targets are
honored; omitting the target uses the current opponent. Players and NPCs use
the same transient timers; automatic attacks, commands and NPC special
procedures respect them. Existing damage-over-time/world effects continue.
This does not enable or rebalance other disabled control powers.

An eleventh regression suite links the actual engine to test casting, combat,
commands, NPC behavior and immunity with synthetic characters only. Test
engine objects live separately from production objects, including under
ASan/UBSan. See the class audit for timing, restart and balance limitations.

## EM-style Vampire teaching and Thaumaturgy

Teaching and the five Path of Blood powers are adapted from Ethereal-Mists
(`EM.zip`, source commit `885604dbca035103ea8bea96e9e40993e2eb00a4`).
Genesis's discipline numbers, nine supported clan choices, generation table,
primal training costs and rank-6/7 availability remain intact. This does **not** import
EM player files, Ghouls, Gargoyles, hybrid classes, or other disciplines.

Teaching is free and unlocks a discipline at untrained rank `-1`; it does not
grant rank 1. A teacher may teach any supported discipline trained to rank 3,
not just clan disciplines. Both players must be connected, visible, standing,
on the same plane and outside combat:

```
teachdisc <student> <discipline>
teachdisc accept <teacher> <discipline>
teachdisc decline
```

The student must personally accept the matching named offer within 120 seconds.
Feeding consent is not teaching consent. Dominate, order, force and nested
interpreter calls cannot create/accept lessons; switched sessions cannot teach.
Acceptance rechecks the teacher, student and discipline. Success is reported
only after the student's atomic save confirms durability. Failed/uncertain
saves retain the offer and roll the in-memory unlock back for a free,
idempotent retry. After a post-rename fsync failure the disk may already contain
the unlock; no XP/primal is ever charged by teaching. Offers are not saved.

| Rank | Command | Effect / cost |
|---|---|---|
| 1 | `tasteblood <target>` | Inspect blood, derived potency, and Vampire clan/generation/trained disciplines; no blood cost, 12-pulse recovery. |
| 2 | `bloodrage <vampire> regenerate` | Enable, never cancel, normal Genesis regeneration. Other players must lower feeding defences to consent. No casting blood cost; regeneration itself pays normal upkeep; 12 pulses. |
| 3 | `potency` | Spend `10 * (14 - clamp(generation, 1, 13))` blood; gain +100 capacity and +1 potency for `min(rank, 5) * random(5, 10)` world ticks; 12 pulses. Does not fill the extra capacity or stack. |
| 4 | `theftvitae [target]` | Steal `random(5, 10) * victim potency`, capped by available blood and free capacity; no upfront blood cost; 12 pulses. |
| 5 | `cauldronblood [target]` | Spend 50 blood; boil up to 10–20 victim blood and inflict that percentage of max HP, reduced by Fortitude; 20 pulses. |

Aliases: `tasteofblood`, `bloodpotency`, `theftofvitae`, `cauldronofblood`.
A named hostile target takes precedence over the current opponent; omitting it
uses the current opponent. Rejected casts do not charge resources or recovery.
Normal safety/PK/plane/combat checks apply before charging. Obfuscation breaks
on successful hostile casts. Genesis's acidic Werewolf/Vicissitude blood can
backfire on Theft. Blood depletion does not execute any class: Theft and
Quietus Leech transfer blood without direct HP damage; Cauldron deals only
its calculated damage, through the existing death handler. Vampires retain
Genesis's torpor/starvation rules. All five Thaumaturgy commands and Quietus
Leech enforce recovery even when invoked indirectly.

**Compatibility adaptations:** base potency derives from Genesis's existing
generation pools: gen 1 = 10; 2–3 = 8; 4–5 = 5; 6–8 = 4; 9–11 = 3;
12–13 = 2; other generations/non-Vampires = 1. No third `blood[]` slot is
introduced. Blood Potency uses one named, serialized capacity affect and a
derived +1 potency bonus; it survives save/load and generation updates, expires
on normal world ticks, and clamps current blood when capacity falls. Existing
skill IDs remain stable; the new skill is appended. Older binaries do not know
the new skill/affect, so rolling back after using it needs explicit save cleanup.
Blood Rage's advertised EM stat-boost option was not implemented there and is
not invented here.

The twelfth suite links the actual engine and covers teaching/forced-consent
guards, existing training costs/caps, save failures (including post-rename
directory-fsync uncertainty), target safety, blood boundaries, backfire,
Fortitude, Vampire torpor, buff expiry and real save/load. These are synthetic
tests, not multiplayer balance proof. If the world has no trained Thaumaturgy
teacher, an operator must seed the initial rank-3 teacher through existing
staff tools; no existing account or clan mapping is changed automatically.

The follow-up [Vampire balance audit](deploy/VAMPIRE_BALANCE.md) compares costs,
timings and damage with the other nine classes. It documents blood conservation,
the removal of last-blood executions, Quietus Leech hardening, and the remaining
high-HP/buff-duration playtesting questions. These are not claims of equal
strength across classes.

## Class-code organization

Class-specific implementations are grouped under `Genesis/src/classes/<class>/`.
The explicit `classes/modules.mk` manifest is shared by production and
real-engine tests; stable saved class IDs live in `classes/class_ids.h`.
All 15 moved class/header files preserve their pre-cleanup contents. No class
costs, ranks, skill slots, player-data layout or hybrid availability change.

See [class ownership and the remaining cleanup stages](deploy/CLASS_LAYOUT.md)
for the ten-class map, shared engine hooks and the separate saved-field
migration work. `make test` now includes ten layout checks alongside
26 C suites; `python3 tests/test_catalog.py` runs fourteen additional catalog checks.
Runtime optimization remains measurement-driven, not an assumed benefit of
file organization.

Runtime ownership cleanup now validates live reciprocal object/familiar links,
unwinds switched sessions before player removal, preserves switched-player idle
activity, and releases staff class-reset links. A real-engine suite exercises
those paths with private players, rooms and socket pairs. Scope, unchanged
behavior and remaining risks: [lifecycle safety](deploy/LIFECYCLE_SAFETY.md).

Switched originals now participate in periodic saves. Primary/backup failures
have independent bounded retries while the character remains loaded, with
truthful manual-save outcomes and a durable-primary checkpoint before logout
cleanup. Policy, failure cases and limitations:
[autosave recovery](deploy/AUTOSAVE_RECOVERY.md).

Manual shutdown/copyover now require durable player checkpoints, and copyover
uses a checked atomic handoff and failure-safe socket flags. The isolated
`python3 tests/smoke_copyover.py` exercises real exec and active TCP recovery.
Upgrade instructions and limits: [restart safety](deploy/RESTART_SAFETY.md).

The sanitizer gate also builds the full server and exercises stock-world
boot, TCP/TERM and real copyover under ASan/UBSan with fatal diagnostics.
High-level NPC HP is capped before unsafe squaring/narrowing. Random ranges
support the full signed-int domain without overflow or a 24-bit ceiling;
ordinary spans retain the legacy RNG sequence and reversed-range behavior.

The first bounded extraction moved six existing Vampire/Werewolf handlers out
of `clan.c`: Majesty, Awe, Humanity, Beastlike, Umbral Vision and Rage. Their
function bodies are byte-identical to the previous head; shared Fangs, Claws,
Nightsight, Calm and Flex stay in the shared module. The seventeenth suite was
run against the old handlers before extraction and exercises the same
real-engine behavior afterward, without loading real accounts.

The next batch extracts Regenerate, Earthmeld, Vanish, Summoning, Possession,
Mirror Image and Control Illusion into Vampire-owned modules. Each of these
seven bodies was compared byte-for-byte with head `0f6598a`, and the eighteenth
full-engine suite passed before and after extraction. It checks rank/class
gates, group invisibility, summoning consent/room restrictions, NPC bindings,
actual illusion creation and the five-follower cap. Shared commands and
transformation commands remain in shared legacy files; no save migration or
numerical rebalance is bundled with the extraction.

A separate hardening change rejects Possession while the caster already has
a familiar, matching Control Illusion's existing rule. Previously retargeting
overwrote the caster's pointer but left the first NPC bound. Regression tests
cover Dominate ranks 3–7, cross-power bindings and rebinding after NPC removal.

## Combat damage wording

Gathering's wording is compressed into Genesis's existing seven damage bands;
no thresholds, damage calculations, verbs, class costs or recovery rules change.

| Damage passed to the formatter | Severity suffix |
| --- | --- |
| 0–50 | like a wimp. |
| 51–250 | lightly. |
| 251–500 | hard. |
| 501–1,000 | incredibly hard! |
| 1,001–2,500 | with ULTIMATE power! |
| 2,501–5,000 | with UNGODLY vengeance! |
| 5,001+ | with UNMERCIFUL strength!! |

Genesis's existing zero-damage formatter behavior remains in the first band;
this does not introduce a miss tier or change the separate avoidance logic.
The extra Gathering tiers are not imported. Numeric damage visibility stays
unchanged for attackers, victims and observers using `PLR_SDAM`.
Message formatting is bounded so long weapon/power names cannot overflow the
local display buffers; a missing name displays as `attack`.
The nineteenth full-engine suite checks both sides of every threshold, all
six damage types, all four recipient modes, unchanged actor resources/state,
and oversized/missing names under normal and sanitizer builds.

## Transformation persistence boundary

The shared, non-saving `reset_form_stats()` helper now owns equipment/stat
cleanup in `transformation.c`. The old `clear_stats()` entry point remains a
reset-and-save compatibility wrapper. All nine transformation call sites use
the non-saving helper, then autosave only after flags, appearance and class
bonuses are complete: Bat, Wolf, Mist, Serpent, Shadow Body and Dragon Form.
Dragon's return to human also saves its completed state.

Previously those handlers could save a new form bit with the old appearance
and missing polymorph/flight/ethereal flags, or human flags with the old form
description. The twentieth full-engine suite first reproduced that disk
round-trip failure. It covers both directions for all six forms, equipment
unequipping, affect removal, Dragon features, rejected class/NPC calls, the
non-saving helper and legacy wrapper, 72 injected save-stage/direction cases,
separately unavailable primary/backup destinations and explicit save retries.

No saved-field format, class IDs, costs, rank gates or numerical bonuses
change. Failed pre-rename writes retain the old file; directory-fsync failures
leave a complete new file visible but durability uncertain. Live gameplay
remains in the completed form rather than attempting unsafe equipment/affect
rollback. Primary and backup are independently atomic, not a two-file
transaction; retries use the existing autosave path. This does not repair
pre-existing inconsistent saves or finish follower accounting, ownership of
overlapping temporary flags, class teardown or hybrid migration.

### Mage self-targeting combat preflight

Native-engine duel preparation reproduced a null-room crash when a Mage
used Better Body on themselves. The shared safety helper no longer removes
a caster from the room for self-targets or other same-room targets, and
still calls the normal safety check. Remote targets retain the existing
temporary move and restoration. Regression cases cover self-targets,
protected self-targets and preservation of same-room character links.
Spell costs, rank gates and numerical effects are unchanged.
