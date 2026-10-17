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

Sources: `clan.c`, `werewolf.c`, `sorcerer.c`, `drow.c`, `highland.c`,
`mage.c`, `dragon.c`, `ninja.c`, `monk.c`, `elemental.c`.
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
- **Stun Fist:** explicitly disabled by an unconditional return in `monk.c`.
  Kept disabled; training display now says so.
- **Quicksand / Lycaon:** explicitly gutted in `wwtribe.c`. No speculative
  implementation or re-enablement.
- **Teach discipline:** `do_teachdisc` in `clan.c` currently returns
  unconditionally. Recorded as unavailable, not silently restored.
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
