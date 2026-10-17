# Class catalog, costs and remaining gaps

This is a targeted source audit, not an exhaustive balance review or a promise
that every registered command has been exercised in a live multiplayer game.
References below are relative to `Genesis/src/`. No historical player data
was used. Existing gameplay constants and save-file identifiers are preserved.

## Training economy

`next` means the current rating plus one; costs are paid only when the
corresponding training command accepts the purchase.

| Class | Training command | Existing cost and limit |
| --- | --- | --- |
| Vampire | `discipline <name>` | Clan discipline: next x 5 primal; other: next x 15. An unlearned -1 rating costs 5/15 and becomes 1. Generation and discipline rules also apply. |
| Werewolf | `gifts <family>` | Preferred breed/auspice/tribe: next x 10 primal, maximum 5; others next x 15, maximum 4. |
| Sorcerer | `colors <color>` | Next x 10 primal; maximum 5. |
| Drow | `prayer <person> <power>` | Individual fixed Drow-point prices displayed by `prayer`; speed costs 10,000 per rank, maximum 4. Gifts to another Drow require the same nonempty house. |
| Highlander | `increase <ability>` | Next x 15 primal; maximum 6. |
| Mage | `sphere <name> learn` | Next x 10 primal; maximum 5 in selected clan/avatar/personal spheres, 4 otherwise. `arete`: next x 500,000 experience, maximum 10. |
| Dragon | `dragonskill <skill/age>` | Skills cost 1,000 x (12 - required age), subject to age gates. Age upgrades have a separate DP/QP schedule in `dragon.c`; do not equate these with primal training. |
| Ninja | `principle <family> improve` | Next x 10 primal; maximum 10. |
| Monk | `monklearn <family> gain` | Next x 10 primal; maximum 8. |
| Elementalist | `elemskill <element> improve` | Next x 10 primal; maximum 8. Focus requires all eight elements at 8, raises one by 2 and lowers its opposite by 2. Training is blocked while focused. |

Sources: shared `Genesis/src/clan.c` and the class-owned files under
`Genesis/src/classes/`, selected by `classes/modules.mk`.
See [class ownership](CLASS_LAYOUT.md) for current paths and remaining coupling.
This table describes progression, not every activation cost. Activation costs,
including costs of failed random attempts, remain ability-specific.

## Raw stat caps and training

Ordinary non-immortal player caps from `act_info.c:stat_max`, in
STR/DEX/CON/INT/WIS order:

| Class | STR | DEX | CON | INT | WIS |
| --- | --- | --- | --- | --- | --- |
| Classless | 8 | 8 | 8 | 8 | 8 |
| Vampire | 10 | 9 | 11 | 9 | 8 |
| Werewolf | 11 | 9 | 10 | 8 | 9 |
| Sorcerer | 9 | 9 | 8 | 11 | 10 |
| Drow | 9 | 11 | 8 | 10 | 9 |
| Highlander | 11 | 9 | 10 | 8 | 9 |
| Mage | 8 | 9 | 9 | 11 | 10 |
| Dragon | 11 | 8 | 10 | 9 | 9 |
| Ninja | 10 | 11 | 9 | 9 | 8 |
| Monk | 9 | 10 | 9 | 8 | 11 |
| Elementalist | 9 | 8 | 9 | 10 | 11 |

`raw_stat` caps stored stats at the class limit; `char_stat` adds separate
temporary/equipment/class effects. Stat training costs the square of the
current raw stat x 10,000 experience through rating 5, and twice that square
above 5. Thus 5 -> 6 costs 250,000 and 6 -> 7 costs 720,000. These existing
values are regression-tested, not rebalanced. NPC and immortal caps differ.

## Fixed in the follow-up audit

- The duplicate `ward` registration hid Homid Spirit Ward behind Mage
  Paradox Ward. `ward` now dispatches Werewolves to Spirit Ward; the explicit
  `spiritward` alias is also registered. Mage `ward` remains unchanged.
- Healing Touch retains its 5,000-mana minimum and rolled healing formula,
  but checks its actual 1.5 x healing cost before granting healing or lag.
  Maximum trained Mantra can require 7,200 mana; insufficient mana no
  longer allows healing followed by a negative mana balance.
- Ninja Vanish (`mienaku`) now charges the 20 Ki required by its check
  and advertised description, rather than silently charging 10.
- Mage ability listings now reveal the implemented third-rank powers at
  rank 3 rather than incorrectly waiting until rank 4.
- Monk training text uses the registered `aurashot` name and labels
  `stunfist` as disabled, not as an available working stun.

## Missing, obsolete and deliberately disabled

- **Malkavian:** no acquisition branch or discipline mapping found. Removed
  from class-selection advertising in the earlier pass; no replacement
  mechanics invented.
- **Stun Fist:** restored in the subsequent guarded-restoration pass described
  below. The historical six-second command-only delay was not re-enabled.
- **Quicksand / Lycaon:** explicitly gutted in `wwtribe.c`. No speculative
  implementation or re-enablement.
- **Teach discipline:** subsequently replaced with guarded EM-style teaching
  in `vampire.c`: free at rank 3, explicit named student acceptance, and
  checked persistence. See the subsequent port notes below.
- **Mage per-sphere descriptions:** legacy `sphere <name>` views still
  contain placeholder prose despite implemented spells. `magepowers` is
  the more useful command; its thresholds were corrected in this pass.
- **Literal skill references:** the structural audit found no unresolved
  literal `skill_lookup("...")` references using the existing prefix lookup
  convention (`blind` resolves to `blindness`). Dynamic names, serialized
  spell slots, help files and world-object references need separate checks.

## Verification boundaries

The Python catalog check rejects duplicate command names and missing command
implementations, and validates literal skill references structurally. It
does not prove a registered command works or that a spell is balanced.
C tests exercise actual stat caps/costs, Healing Touch affordability,
Ninja Vanish Ki charging, Werewolf ward dispatch and Mage display thresholds,
in addition to the earlier class/power tests. External gameplay helpers use
synthetic fixtures/stubs; no real accounts are loaded.

## Guarded Stun Fist restoration

`stunfist [target]` requires Monk Technique 7 and 5,000 mana. A named target
is honored even during another fight; without a name it uses the current
opponent. It establishes combat through the existing safety/combat helpers.
Self, missing, sleeping/incapacitated, protected, charm-master and temporarily
immune targets are rejected without spending mana or applying recovery.

At the configured four pulses per second, the target's actions and attacks
pause for four pulses (about one second); the Monk recovers for eight pulses
(about two seconds). The target gains 24 pulses (about six seconds, measured
from application) of immunity to **Stun Fist**, shared across all attackers.
That leaves about five seconds of immunity after the interruption ends.
Rejections do not refresh timers, and a paused/recovering target cannot be
paused again by this ability. Other historical control powers are not changed
and do not share this immunity. No damage or random failure roll is added.

These timers are transient, separate from position and ordinary command lag.
Both main loops tick every loaded character, including NPCs and link-dead
players, once per pulse before input. The interpreter, automatic violence,
multi-hit/single-hit entry points, NPC special procedures and NPC aggression
honor the pause. Timers do not suspend regeneration, already-applied effects,
world events or incoming damage. They are not saved; restarting or unloading
a character resets them. This is not a new generic stun system for all powers.

The eleventh C suite links the full production engine with only its server
entry point renamed. Synthetic player/NPC fixtures exercise actual casting,
safety, combat initiation, attack suppression/resumption, command processing,
NPC special procedures, immunity across different attackers, exact mana
boundaries, targeting and timer expiry. No historical accounts/world files
load. Catalog tests also check main-loop/action-path wiring structurally.
Live multiplayer balance and long-running sessions remain unverified.

## EM-style Vampire port

The dormant teaching routine (including its invalid `cland[3]` access) is
removed. Teaching now unlocks at `-1`, requires a live rank-3 teacher, has no
XP/primal fee, and requires the student's own acceptance of a named, short-lived
offer. Neither broad feeding consent nor Dominate/force can substitute for it.
The learner's checked atomic save must succeed before success is announced;
uncertain saves can be retried without any charge.

Thaumaturgy's old `Paths` advertisement is replaced with actual rank-1–5
commands: Taste Blood, Blood Rage (regeneration only), Blood Potency, Theft of
Vitae, and Cauldron of Blood. The new capacity affect is appended to the skill
catalog without moving existing skills. Potency is derived from Genesis's
generations, not an imported EM blood-array slot. Capacity survives save/load
and generation changes, expires normally and clamps excess blood. Detailed
syntax, costs, durations, portability adaptations and rollback caveats are in
the README.

The twelfth suite links production engine objects, exercising these abilities
and real character serialization using synthetic fixtures and a private
temporary runtime. It includes forced teaching, expiry/decline/revalidation,
unchanged primal training/caps, pre-rename save failure and post-rename
directory-fsync failure, protected/wrong-plane targets, explicit target
selection, capacity boundaries, acid backfire, Fortitude, torpor, and actual
world-tick buff expiration. New powers still require multiplayer balance
testing; this port does not enable missing disciplines or add a Tremere clan.

## Blood-power cost and balance follow-up

See [VAMPIRE_BALANCE.md](VAMPIRE_BALANCE.md) for the source-formula comparison
against the other nine classes and remaining tuning questions. Removed the
arbitrary-HP execution when Theft/Cauldron depleted a non-Vampire's blood.
All five Thaumaturgy commands now reject recasts during existing command lag,
including indirect interpreter dispatch.

The older Quietus-6 Leech had the same depletion execution, could ignore an
explicit target while fighting, waste drained blood against a full caster, and
subtract blood below zero on acid backfire. It now shares the bounded, safe
drain path. It retains rank 6, PC-only targeting, a 10–20-point normal drain,
12-pulse recovery and a fixed 10,000-HP Vicissitude backlash; backlash now goes
through normal combat/torpor handling and cannot skip recovery. This is not a
global rewrite of legacy feeding, command lag or every class's attack handlers.

The existing twelfth engine suite now also covers indirect/cross-power recasts,
all 19 positive recovery values after a Cauldron cast, non-Vampire/NPC blood
depletion without HP execution, one-point Cauldron damage at one-million HP,
Quietus targeting/acid/capacity protections, resource conservation across
valid and malformed generation values, and a six-cast generation-9 budget.
The seeded engine RNG also runs 1,536 near-limit cases: random amounts must be
evaluated once before `UMIN`, whose double evaluation could otherwise overfill
the caster or overdraw the victim. This regression reproduced the pre-fix bug.

## All-ten-class runtime follow-up

The source catalog contains 580 registered command names (547 distinct
handlers) and 113 named skill records. The 15 primary class source files
contain 259 command-handler definitions; shared abilities in `more.c`, magic,
combat and class/stat helpers also matter. Catalog counts are not claims that
every command has been playtested. The audit examined registration, advertised
names/ranks, resources and lifetime-sensitive attack paths across all classes.

| Class | Follow-up findings and coverage |
| --- | --- |
| Vampire | Corrected advertised names (`beastwithin`, `horrid`, `empathy`, `malleable`), several unlock ranks, and the missing Potence-7 `fist` listing. Object-form recovery covered below. |
| Werewolf | Fixed Falling Touch's victim access after lethal damage; marked gutted Quicksand/Lycaon disabled in listings. `powers` no longer appends an erroneous “Huh?” after the Werewolf listing. |
| Sorcerer | Existing safety and independent AoE damage regressions retained; no additional confirmed class-specific defect in this pass. |
| Drow | Acid Bolt respects explicit/invalid names. Crossbow's scoped ranged view restores origin on protected/missing targets, extraction and missing sleep skill, without movement hooks cancelling stealth/darkness/silence. |
| Highlander | Bash checks self/safety before combat and applies victim lag before potentially extracting damage. Explicit names do not silently fall back to another opponent. |
| Mage | Inferno sets burning before potentially extracting damage and computes per-target NPC damage independently. Existing payment/remote-target regressions retained. |
| Dragon | Breath, tail and dust respect explicit/invalid names instead of silently choosing a current opponent. |
| Ninja | Trip/circle no longer fall back to a current opponent for an invalid explicit name. Existing bomb safety and Vanish charging tests retained. |
| Monk | Repeat stops after target extraction or caster relocation and pays 600 mana per attempted hit, not after damage. Aura Shot/Cripple honor explicit names. |
| Elementalist | Icicle Storm pays its existing 150 x Water cost and rejects unsafe/self targets before payment. Flare Arrow, Wind Blades and Stoneblast revalidate chained targets; Stoneblast stops if the caster relocates. Seven targeted handlers honor explicit names. |

These fixes preserve progression caps and existing activation amounts, except
that formerly missing payments now occur. The thirteenth C suite exercises
actual class handlers with synthetic service contracts: lethal NPC unlink/free,
caster relocation, surviving multi-hit counts, exact resource boundaries,
explicit/missing targets and crossbow restoration. Mage tests also free an
Inferno victim inside its damage callback.
Crossbow retains its legacy temporary room-pointer view internally; replacing
that with a dedicated ranged-combat API remains a future refactor. Shots do
not physically move the caster or invoke departure/arrival effects.

The existing full-engine Vampire suite now covers 85 Cauldron envelopes:
all ten target classes, five max-HP tiers (10k, 50k, 100k, 1m and `LONG_MAX`),
and Vampire Fortitude 0–7. It verifies bounded arithmetic, defense ranges and
50-blood/20-pulse payment, not equal PvP power or sustainable DPS.

### Remaining-risk work completed

Read-only examination of the 60 area files named by the bundled `area.lst`
found no Hell room 30000. Plasma/Mummify could remove a character from the
object room before attempting to enter that missing room. They now resolve
Hell, then the configured temple, **before** destroying the body; if neither
exists, they leave form/body intact and report the problem. Full-engine tests
cover both destinations, no destination, body extraction, ownership mismatch
and wrong-form attempts. A bodyless but still active form can be reverted.
The literal class object/mobile/room references are checked against the
loaded area files; computed vnums remain outside that structural check.

Control Weather/Energy Drain had empty but formerly player-callable handlers.
After checking EM, both received the guarded adaptations documented in
[RESTORED_SPELLS.md](RESTORED_SPELLS.md), rather than speculative replacement
mechanics. Armor, Bless, Shield and Stone Skin were subsequently restored with
the limited adaptations documented in RESTORED_SPELLS.md. The remaining
17 empty legacy spells stay at unavailable level 99;
the catalog check prevents advertising an empty spell as player-castable.

### Still unverified or deliberately unresolved

- Quicksand/Lycaon and Malkavian mapping are not restored; Mage sphere prose
  still contains placeholders.
- Full multiplayer balance, sustained regeneration/feeding, equipment-heavy
  matchups and generic indirect lag behavior outside the guarded powers remain.
- Combat resource transfers are not multi-character durable transactions.
  Post-rename fsync uncertainty and crash/power-loss testing remain.
- Initial Thaumaturgy teacher seeding requires an operator; no arbitrary
  player is granted that discipline automatically.
- Dynamic world references, legacy spell objects, privileged commands,
  non-player persistence and long-running sessions are not exhaustively tested.
- CI's systemd verification does not start a live service or validate a real
  host's sandbox, permissions, firewall, backup/restore or rollback procedure.
