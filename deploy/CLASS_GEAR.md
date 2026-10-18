# Class gear audit

This audit covers the ten supported single classes. It does not introduce
hybrids or new object templates. Permanent gear prices remain unchanged; the
Drow ammunition follow-up adds the documented 100-point bolt recipe.
The reference world data is
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
| Drow | `drowarmor bolt` / `drowarmor bolts [1-20]` | 100 Drow points per bolt; no quest, gold or primal charge | 29659 |
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
Catalog checks validate all 28 craftable relic/ammo prototypes, weapon value ranges,
Sorcerer material presence and Drow ammunition metadata in the bundled archive.
The full regression and sanitizer suites are still necessary.

## Drow ammunition follow-up

`shoot <direction> <target>` now spends **500 Drow points plus one compatible
loose projectile** from the shooter's direct inventory. A crossbow must be held
in `WEAR_HOLD`. No reload command, magazine counter or stack quantity is
introduced: one `ITEM_AMMO` object is one shot. Existing damage scaling and
sleep-poison behavior remain; ammo damage fields do not introduce a new damage
formula. Damage-roll multiplication is bounded to `number_range`'s int range.

The native Drow crossbow is template **29658** and its bolt is **29659**.
The reference world's crossbow still points to legacy ammo 9315. New crafting
corrects that known reference without altering the prototype; already-saved
native crossbows resolve it at firing time without rewriting saves. Other
configured ammo vnums remain supported. Projectile type must match the weapon.

Only unworn, directly carried ammunition is selected. Unowned world ammunition
and the shooter's own ammunition are allowed; foreign-owned, artifact,
character-shaped, content-bearing, worn and bagged objects are not consumed.
Malformed weapon metadata and absent/incompatible ammo fail without payment.

Bad syntax/directions, same-room targets, insufficient points, no held crossbow,
source-room safety and protected visible targets do not consume ammo or points.
Actual shots into empty rooms, nonexistent exits, closed doors or magical
granite/water/wind/flame walls consume one projectile and 500 points. Magical
walls now block the shot rather than allowing it to pass through.
The shooter is never physically moved: room membership and stealth/darkness/
silence remain intact on every exit path. The existing damage/poison safety
checks and lethal-target cleanup remain in force.

### Player-crafted supply

Players can now create their own supplies:

```
drowarmor bolt
drowarmor bolts 10
```

The default is one bolt; batch size is 1–20, at **100 Drow points per bolt**.
There is no quest-point, gold or primal charge. Ten bolts cost 1000 points;
firing one still costs 500 points, so a freshly crafted-and-fired bolt costs
600 points in total. This is an initial balance setting, not a measured
multiplayer balance result. Permanent Drow equipment remains 8000 points
plus 400 quest points.

Crafted bolts record their maker but leave ownership empty, allowing Drow
players to supply one another. Their shop resale value and refundable quest
points are zero, including after saving/reloading. Foreign-owned ammunition
remains protected by the existing firing checks.

Invalid quantities, insufficient points, missing/wrong-type/non-takeable
templates, invalid weight/caliber and overflowing carry counters fail before
creating any items or charging any points. Batches are bounded to 20;
partial batches are not created for insufficient funds. Crafting uses normal
player-save timing, not a new independently durable transaction.

**Rollout prerequisite:** the existing bolt template 29659 must be loaded and
valid (`relic.are` in the supplied world). Manual shop/loot stocking is no
longer required: the player recipe supplies bolts. No shop prices, loot resets,
world-archive edits or player-file migrations are installed. Back up world and
player data before rollout; players still need to craft or obtain bolts to fire.
The recipe always creates native bolt 29659. Crossbows deliberately customized
to a different ammunition vnum still need their configured ammunition supplied.

## Conjured consumables and market follow-up

All ten class directories and shared player object-creation paths were traced.
The confirmed renewable-resource resale paths were Sorcerer supplies,
Vampire Shadowforge weapons, and equipment created for mirror images.
Ordinary `cloneobject` already assigns zero value; the shared market now also
rejects all `ITEM_VANISH` illusions, including legacy mirror equipment.

| Class/path | Resource rule preserved | Economy/lifecycle result |
|---|---|---|
| Vampire Shadowforge | Obtenebration 6, 50 blood; slash/pierce/blunt, timer 2700 | Zero shop/refund value; old saved native 29639 weapons also refused by shops |
| Vampire mirror / Vampire-Werewolf illusions | Existing prerequisites and costs | Mirror equipment created without monetary/refund value; all illusory objects refused on both shop sides |
| Sorcerer page/book/pen | Page free; book/pen 10 primal | Zero monetary/refund value; old 30037 writing supplies also refused by shops; tradability unchanged |
| Werewolf Faerie Light | Fianna 1, no new charge | Zero monetary/refund value; missing/wrong-type template or room fails safely |
| Werewolf Fetish Doll | Uktena 5; one matching held body part | Validate doll template/recovery room; preserve valid severed heads but protect unrelated shaped/containing/artifact objects; zero monetary/refund value |
| Drow bolts | 100 Drow points per bolt, firing +500 | Existing bounded recipe and zero-resale safeguards retained |
| Mage / shared soulblade, light, food, spring spells | Existing cast prerequisites/mana charges | Created items have zero monetary/refund value; validate templates; bound malformed extreme food/spring levels |
| Ninja, Monk, Elemental, Highlander, Dragon | Existing abilities and permanent gear costs | No additional class-owned physical-consumable factory found; Ninja bomb attacks do not manufacture a saleable bomb object |

Shop valuation rejects negative/overflowing prices/profit factors, invalid
staff/wand charge counts and shaped-character objects. Buying/selling checks
gold arithmetic before moving items or charging. Selling keeps remaining item
lifetimes instead of making timed goods permanent, and replaces the cleared
ownership string safely rather than retaining a freed pointer.

This is not a blanket repricing of world stock or paid permanent relics.
Older ordinary food/light/spring/soulblade objects are not retroactively
rewritten or identified as conjured: their templates may also be used by
operators for legitimate world stock. The dedicated Shadowforge/writing-supply
and illusion checks protect those identifiable legacy cases without a save
migration. Staff creation, quest-card pricing/rewards and general currency
transfers were not redesigned.

Real-engine gear regressions cover new/legacy supply valuation, actual sales,
refund and save/reload boundaries, timer/owner preservation through sell/buy,
player/shop gold overflow, malformed wand/staff charges, missing/wrong-type
templates, protected body parts and successful one-part/one-doll creation.
Valid reciprocal severed heads retain the existing final-death/altar recovery
behavior; a missing altar now fails before the head is consumed.
Held reciprocal heads can now resolve their player even when normal character
lookup hides severed characters; unrelated object-character links cannot
bypass lookup.
Live multiplayer balance, power-loss behavior and comprehensive economy/soak
testing remain separate work; crafting retains normal player-save timing.
Fetish matching still uses the legacy part-name/value metadata; this pass does
not add a new body-part provenance or save format. The unsafe use of an item
name as a printf format string has been removed.

`test_drow_ammo` exercises the real engine's consumption, eligibility, ownership,
custom/legacy compatibility, all five barrier types, misses, safety boundaries,
room/stealth invariants, crafting and old-save/remaining-ammo persistence.
Recipe tests cover 1/20 limits, full-batch payment, invalid and overflowing
quantities, template/carry-counter failures, tradability, zero shop value,
zero quest refunds and preserved maker/economy fields across save/reload.
The synthetic runtime suite separately covers lethal target removal,
unavailable poison skills and negative/extreme damage-roll bounds.

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
