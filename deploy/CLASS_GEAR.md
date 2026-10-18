# Class gear audit

This pass checks the ten supported single classes. It does not introduce
hybrids, new object templates, or new prices. The reference world data is
`Genesis.zip`; operators must verify their installed areas separately.

## Crafting inventory and existing prices

| Class | Command / items | Cost per creation | Object vnums |
|---|---|---|---|
| Vampire | `varmor`: mark, trenchcoat, vest | 100 primal + 400 quest | 29630–29632 |
| Vampire | `shadowforge`: slash, pierce, blunt; Obtenebration 6 | 50 blood; temporary weapon, timer 2700 | 29639 |
| Werewolf | `wwarmor`: vest, mask, talisman | 100 primal + 400 quest | 29640–29642 |
| Ninja | `ninjaarmor`: belt, boots, gi, cloak; Chikyu 1 | 100 primal + 400 quest | 29600–29603 |
| Monk | `monkarmor`: necklace, habit, belt, tiger, dragon | 100 primal + 400 quest | 29610–29614 |
| Elemental | `elemarmor`: bracer, sword | 100 primal + 400 quest | 29620–29621 |
| Drow | `drowarmor`: cloak, helmet, breastplate, bracer, crossbow | 8000 Drow points + 400 quest, not primal | 29652–29655, 29658 |
| Highlander | `forge`: create or reforge an owned, unworn, non-relic/non-artifact weapon | 50 primal + 100 quest | 29650 |
| Mage | `magearmor`: robes, hood, talisman | 100 primal + 400 quest | 29660–29662 |
| Sorcerer | `sorccreate`: book, pen / page | 10 primal / free | uses protoplasm template 30037 |
| Dragon | No dedicated class-relic crafting command/set | Not invented by this audit | Natural-form equipment rules remain |

Mage `mageenchant` requires Prime 3 and an unworn ordinary weapon. An attempted
cast costs 5000 mana and five quintessence, including failed casting rolls.
Heart's blood can replace a missing portion of those five points: available
Body quintessence is spent, and each missing point costs 200–500 HP. The caster
must have more HP than the maximum possible self-damage before payment;
invalid resources, targets, already-aggravated weapons and insufficient mana
are rejected without charging. Relics and artifacts remain ineligible.

Sorcerer `enchant` retains its existing zero-cost, once-only +5 hitroll / +5
damroll behavior for eligible unworn armor/weapons. This pass does **not**
invent a price or newly forbid relics. Sorcerer writing supplies are not a
dedicated class-relic set. Consumables retain their existing tradability.

## Corrected bugs

- Highlander reforging copies exactly four values, not a fifth value beyond
  both object structures. Regression uses an exactly sized real engine
  free-list object so ASan can detect the arena-hidden original overrun.
- Missing Sorcerer material templates return an error without charging,
  rather than calling `create_object(NULL)` and exiting the server.
- Mage enchantment actually deducts quintessence and checks mana before
  inflicting heart's-blood damage. Negative quintessence is rejected.
- Werewolf's invalid-choice message lists the three implemented recipes,
  not six nonexistent ones.
- Equipment-owned reflection, artifact and hand-specific silver flags are
  rebuilt from worn gear even when an effect already exists. This prevents
  duplicate-effect early returns from bypassing the single-artifact rule.
- Magic resistance is recomputed from worn Drow/Highlander gear, one reflection
  bonus and innate Dragon form, retaining the existing 75% cap and coefficients.
  Removing one item no longer subtracts from a capped aggregate incorrectly.
- An item effect is retained while another worn item grants the same effect,
  including equivalent spell IDs such as sanctuary 8/57.
- Full player loading and Dragon transitions refresh equipment state. Naked
  Dragon forms recover innate resistance on load; leaving form retains any
  remaining equipment contribution.
- Roomless character cleanup no longer dereferences a nonexistent room when
  unequipping a loaded item and broadcasting its removal.

## Tested scope and remaining limits

`test_class_gear` uses the full engine, synthetic private characters and
templates: 29 crafting variants, both resource boundaries, missing templates,
class restrictions (including unclassed, hybrid and NPC rejection), reforging,
enchantment payment, overlapping effects, artifact limits, silver hands,
Dragon persistence with and without gear, and roomless item cleanup.
Catalog checks validate all 27 craftable relic prototypes, weapon value ranges,
Sorcerer material presence and Drow ammunition metadata in the bundled archive.
The full regression and sanitizer suites are still necessary.

Existing Drow `shoot` spends 500 Drow points and implements its projectile
directly. It does not consume inventory ammunition or use the missile template's
ammo-vnum field. The reference crossbow points to ammo 9315; a separate Drow
bolt template 29659 also exists. This is an unfinished/legacy ammunition
integration, not a new free bolt recipe. Misses and blocked shots retain their
existing firing costs.

Sorcerer and Dragon have no dedicated relic crafting sets; this is a content
gap, not an excuse to fabricate item stats or serialized class flags.
Highlander reforging still follows its legacy conversion rules rather than
merging every source item's affects and metadata. Unclaimed worn generic gear
still follows the existing save exclusion; claim/ownership policy is unchanged.

The engine still shares several flags between intrinsic abilities, temporary
spells and item effects. Retaining another item source is covered here;
complete source ownership across unrelated abilities, dispels, transformations
and every teardown path is not. No live-world balance study, multiplayer soak,
production deployment or server power-loss test has been performed.
