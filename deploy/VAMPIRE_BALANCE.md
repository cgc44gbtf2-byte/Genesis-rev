# Vampire blood-power cost and balance audit

Scope: the EM teaching/Thaumaturgy port and overlapping Quietus Leech.
This is a source-formula audit plus synthetic full-engine regression testing,
not a multiplayer balance simulation or a declaration that all classes are
equally strong. Existing training costs, clan mappings and rank limits remain.

## Fixes, not speculative tuning

- Theft no longer kills a healthy non-Vampire on the last blood point.
  It transfers a resource; it does not directly deal HP damage.
- Cauldron no longer replaces its damage with the target's remaining HP on
  blood depletion. One remaining blood point means 1% max-HP damage before
  Fortitude, not an arbitrary-HP execution.
- Each Thaumaturgy handler rejects positive command lag itself. Nested
  interpreter calls cannot bypass recovery, switch to a second blood power,
  or refresh the recovery timer by repeatedly making rejected casts.
- Drain and boil amounts roll once before clamping. The legacy `UMIN` macro
  may evaluate its argument twice; an inline random roll could exceed the
  previously checked capacity/available-blood limit. A seeded engine regression
  reproduced overfilling before this fix.
- Quietus-6 Leech now shares the safe, capacity-bounded drain path. Explicit
  names beat the current opponent; full casters cannot waste a victim's blood.
  Acid cannot subtract blood below zero, skip recovery, or bypass normal
  combat/torpor processing. Last-blood executions are removed here too.
  Its rank, PC-only restriction, 10–20 normal drain, and fixed 10,000-HP
  Vicissitude backlash remain; the latter can still torpor its caster.

The normal drain conserves the two characters' combined current blood.
Acid blood deliberately destroys blood instead of granting a positive transfer.
Theft retains its Fenris counter; Quietus retains its original Vicissitude
counter, without adding Fenris as a new Quietus counter.

## Thaumaturgy costs and actual effects

Four server pulses are one nominal second. Ordinary command lag and spell
world-tick durations are different clocks.

| Rank | Power | Resource cost | Recovery / effect |
|---|---|---|---|
| 1 | Taste Blood | None | 12 pulses / 3 s; inspection only |
| 2 | Blood Rage | No cast fee | 12 pulses / 3 s; enable existing Vampire regeneration; another player must have lowered feeding defenses |
| 3 | Blood Potency | `10 * (14 - clamp(generation, 1, 13))` blood | 12 pulses / 3 s; +100 capacity, +1 derived potency; no stacking or free blood |
| 4 | Theft of Vitae | No upfront fee | 12 pulses / 3 s; drain `random(5,10) * victim potency`, bounded by available blood and caster capacity |
| 5 | Cauldron of Blood | 50 blood | 20 pulses / 5 s; boil up to 10–20 target blood; that percentage of target max HP before Fortitude |

Potency is not a general damage, regeneration or attribute bonus. In this port,
its derived value is inspected by Taste Blood and weights the *victim's* blood
when Theft drains it. Increasing one's own potency also makes one's blood more
valuable to an enemy thief. Capacity only helps after acquiring extra blood.

Potency duration is `min(5, trained rank) * random(5,10)` world ticks:
15–30 at rank 3, 25–50 at rank 5. A world tick is nominally 60 seconds, not a
combat pulse; the existing affect engine removes an effect on the tick after
its duration reaches zero. Offline durations follow existing save/load rules,
not a new wall-clock expiration.

Blood Rage sets the existing regeneration mode; it is not a new ten-tick buff
despite the internal nonzero value being 10. Existing Vampire regeneration
does not decrement that counter. When healing is needed, a normal regeneration
update costs 4 blood (20 under Thin Blood) and doubles the normal heal
(1.5 times under Thin Blood). The ordinary mobile-update cadence is two
seconds; a healthy Vampire does not pay this healing cost. Existing passive
Vampire starvation/upkeep is separate.

With a full unbuffed generation-9 pool of 300, six 50-blood Cauldron casts
are affordable before other spending. Casting Potency first costs 50 and
leaves 250 current blood, not 400: five Cauldrons without refilling.
Refilling its raised 400 capacity would permit eight. This is a resource
budget, not a duel prediction: targets, healing, melee, defenses and starvation
still matter. Normal teaching is free; non-clan ranks 1–5 still cost a total
of 225 primal, versus 75 for a clan discipline.

## Cross-class reference points

These are source-level command components before full combat resolution.
They do not normalize equipment, stats, avoidance, resistances, secondary
effects, AoE, botches, upkeep, resource recovery or ordinary melee attacks.
Blood and mana are not interchangeable units. A `one_hit` call uses the shared
attack engine and is not a fixed damage number.

| Class / example | Command-local cost | Recovery | Damage component |
|---|---|---|---|
| Vampire / Cauldron | 50 blood | 5 s | 10–20% target max HP with enough target blood, before Fortitude |
| Monk / Aura Shot at Tech 7 | 2,000 mana | 3 s | 1,400–2,100 before avoidance; also heals its caster |
| Mage / Electrical Chaos | 3,500 + `100 * correspondence^2 + 300 * quint` mana, plus quint | 2 s | `random(1000,1200) * power`; power depends on Arete rolls and modifiers; resistance/failure apply |
| Elementalist / Flare Arrow | `100 + 50 * Fire` mana | `(20 - Fire) / 4` s | Three `one_hit` calls; possible burning |
| Ninja / Shuurai | No explicit command-local spend | 2.25 s | One `one_hit` circle attack; Ningenno 4 required |
| Drow / Acid Bolt | No explicit command-local spend | 2 s | One `one_hit` acid-bolt attack |
| Dragon / Breath | No explicit command-local spend | 2 s | One `one_hit` breath attack; dragon form required |
| Highlander / Bash | No explicit command-local spend | 5 s | 851–950 damage component; Ability 3 required |
| Werewolf / Falling Touch | No explicit command-local spend | 6 s | `random(125,250) * Stargazers`; also imposes victim lag |
| Sorcerer / Red chant | 600 mana before chant modifiers | 3.75 s | `random(140,180) * Red rank` before book/chant modifiers |

Source: [vampire.c](../Genesis/src/classes/vampire/vampire.c),
[monk.c](../Genesis/src/classes/monk/monk.c), [mage.c](../Genesis/src/classes/mage/mage.c),
[elemental.c](../Genesis/src/classes/elemental/elemental.c), [ninja.c](../Genesis/src/classes/ninja/ninja.c),
[drow.c](../Genesis/src/classes/drow/drow.c), [dragon.c](../Genesis/src/classes/dragon/dragon.c),
[highland.c](../Genesis/src/classes/highlander/highland.c), [wwtribe.c](../Genesis/src/classes/werewolf/wwtribe.c),
[sorcerer.c](../Genesis/src/classes/sorcerer/sorcerer.c),
[update.c](../Genesis/src/update.c), [clan.c](../Genesis/src/clan.c),
[act_move.c](../Genesis/src/act_move.c), and [merc.h](../Genesis/src/merc.h).

## Tuning questions intentionally left open

- Cauldron scales from 1,000–2,000 raw damage at 10,000 max HP to
  10,000–20,000 at 100,000 max HP. Normal HP training reaches 100,000.
  Fortitude reduces it by a random `5 * rank` to `10 * rank` percent,
  with rank clamped to 0–7. It does not use the Mage ATM resistance roll.
  Test high-HP and non-Fortitude matchups before deciding on a cap, different
  counter, percentage reduction or cost increase; a static comparison alone
  is insufficient to select that parameter.
- Theft against generation-9 Vampires normally transfers 15–30 blood versus
  5–10 against non-Vampires. Quietus Leech transfers 10–20 against PCs
  regardless of generation. Thus the two drain powers overlap, with different
  useful targets; no arbitrary Quietus damage bonus was invented.
- Potency is a long-lived capacity/tradeoff buff, not a combat steroid.
  Duration/cost and the extra vulnerability to Theft need player feedback.
- Regeneration, feeding, equipment and group combat affect sustained blood
  income. An integer-conservation test does not establish sustained DPS.
- Existing legacy feeding and other classes' indirect command-lag behavior
  are not globally rewritten by this focused patch.
- Atomic player saves are not a two-character transaction for each combat
  transfer. Crash/reload behavior and long multiplayer sessions remain risks.

## Regression coverage

The existing twelfth C suite, [test_vampire.c](../tests/test_vampire.c), links
the actual engine. Added checks cover indirect and cross-power recasts, every
positive lag value below Cauldron's 20-pulse recovery, unchanged timers after
rejection, casts after recovery, blood depletion without HP execution,
one-point Cauldron damage at one-million max HP, NPC Theft depletion,
Quietus rank/target/capacity/acid safety, transfer conservation across eleven
valid/malformed generation cases, and the six-cast generation-9 budget.
The engine RNG is initialized with a fixed seed; 1,536 repeated near-limit
drain/boil cases exercise interior random-range boundaries, not an uninitialized
all-zero RNG that would always select the minimum.
Original teaching, save-failure, consent, target protection, buff serialization,
world-tick expiry and combat/torpor checks remain.

The all-class follow-up adds 85 full-engine Cauldron envelope cases covering
all ten classes, max HP 10k/50k/100k/1m/`LONG_MAX`, and Vampire Fortitude 0–7.
These verify damage/resource bounds and extreme arithmetic, **not** a live
duel, sustained DPS or a decision that percentage damage needs no tuning.
