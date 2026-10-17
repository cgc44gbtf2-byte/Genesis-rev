# Guarded EM spell adaptations

Reference: Ethereal Mists, `EM/src/magic.c`, revision
`885604dbca035103ea8bea96e9e40993e2eb00a4`, in
[EM.zip](https://github.com/cgc44gbtf2-byte/Ethereal-Mists-rev/blob/885604dbca035103ea8bea96e9e40993e2eb00a4/EM.zip).
Existing copyright notices and the README's licensing limitations still apply.
These are Genesis adaptations, not a promise of identical EM gameplay.

Both spells previously had empty Genesis handlers. Their skill indices and
historical object-spell slots (11 and 25) are unchanged. Availability is
`LEV_AWA` (awakened, level 2), not EM's different level-3/4 progression.
They use the existing generic spellcasting system across classes; no new
discipline, sphere, movement pool, immunity bit or save-file field is added.

| Syntax | Mana | Base recovery | Effect |
| --- | --- | --- | --- |
| `cast 'control weather' better` / `worse` | 25 | 12 pulses | Changes the global weather trend, not immediate sunlight or instant rain. |
| `cast 'energy drain' <target>` | 35 | 12 pulses | Modest spell damage, 1–10 target mana drained, and bounded healing from confirmed HP loss. |

At four pulses per second, 12 pulses is three seconds **before** the existing
Intelligence/class discounts. These restored casts always retain at least
one recovery pulse. Existing Wolfman mana doubling also applies. Invalid
weather arguments, missing/self/protected/wrong-plane Energy Drain targets,
insufficient mana, Stun Fist pause and positive command recovery are rejected
before spending cast mana. Resisted offensive attempts still spend mana and
recovery and can provoke normal retaliation.

## Weather boundaries

EM's `dice(level / 3, 4)` trend adjustment is retained, with at least one die.
Effective spell level is clamped to 1–100. Existing trend is normalized to
-12..12 before adding a bounded roll, so even repeated casts cannot accumulate
an overflowing counter. The ordinary weather tick still clamps the resulting
change to -12..12 and advances pressure. The regression suite exercises that
real weather update, not only the handler's variable assignment.

Weather-changing objects have no better/worse argument in the generic item
casting interface; they cannot choose a direction. This port does not redesign
item consumption or add a default direction.

## Energy Drain boundaries

- EM's ordinary one-die damage is retained: 1..effective spell level, clamped
  to 1..100 and initially limited to remaining target HP. Normal Genesis
  damage modifiers, reflection and combat safety still apply; this is not a
  universal 100-HP cap after modifiers.
- Saving throws and player ATM resistance are honored. EM's caster-HP-based
  attack against level-2 targets is **not** imported: level 2 is a normal
  Genesis player, not a disposable low-level target.
- Only confirmed HP loss to a surviving target in the original room grants
  benefits. Reflected damage, extraction, incapacitation/death or relocation
  cannot grant a speculative heal or resource transfer. Kills grant no
  Energy Drain healing; ordinary kill processing is unchanged.
- Mana drain is bounded by available positive mana. It is destroyed, not
  transferred to the caster. Genesis has no movement pool to drain.
- Healing is limited to confirmed HP loss and missing HP, never EM's twice-
  maximum-HP overheal. There is no new alignment penalty or Hero/Demon
  exemption, and no invented `IMM_DRAIN` bit.

## Callback lifetime hardening and tests

`damage`, normal cast retaliation and item cast retaliation validate both
participants against a cached room's live membership before touching them
after callbacks. This is a room-sized scan, not a whole-world character scan.
Item recovery is applied before callbacks so an extracted NPC caster cannot
be accessed to apply lag afterwards.

The fourteenth C suite links the real engine, with one dedicated test build
of `fight.c` exposing a compile-time-only damage callback injection. All
normal saves, safety, reflection, casting, interpretation and weather updates
remain real. The injected callback unlinks and **frees** synthetic NPCs,
including an NPC caster killed after Stargazer reflection, so ASan can detect
stale accesses that the engine's recycling allocator could conceal. This
injection is absent from production and the other engine suites.

Live PvP tuning, sustained resource income, long sessions and generic legacy
item consumption still need operator/playtester validation. The remaining
17 empty legacy spell handlers stay unavailable through ordinary player
casting; their identifiers are not removed or reassigned.

## Basic protective buffs

The subsequent archive review found corresponding implementations in every
reviewed archive. The first limited batch restores **Armor, Bless, Shield and
Stone Skin**, using existing Genesis spell indices, object slots, affect
records and saved modifier fields. No new save format or class ID is added.
Reference formulas were compared against the uploaded GodWars 2k6
`godwars/src/magic.c`; the original notices and licensing limitations still apply.

| Syntax | Mana | Base recovery | Effect | Duration |
| --- | ---: | ---: | --- | --- |
| `cast armor [target]` | 5 | 12 pulses | +20 armor | 24 world affect ticks |
| `cast bless [target]` | 5 | 12 pulses | +B hitroll and +B to the generic saving threshold | 6 + L affect ticks |
| `cast shield [target]` | 12 | 18 pulses | +20 armor | 8 + L affect ticks |
| `cast 'stone skin'` | 12 | 18 pulses | +40 armor, self only | L affect ticks |

Here L is effective spell level clamped to 1..100, and B is
`max(1, floor(L / 8))`, therefore 1..12. Normal casts derive level from the
existing bounded Intelligence formula. Wolfman mana doubling, class recovery
discounts and the minimum one-pulse recovery still apply. Durations are
**world affect ticks**, not seconds or recovery pulses. Existing expiry removes
duration-zero affects on the following affect update.

### Genesis-specific corrections

- Higher armor increases Genesis avoidance. Copying the donor's negative
  armor modifiers would make these buffs penalize defense, so the signs are
  positive. Actual combat uses integer armor/250 and hitroll/150 thresholds;
  a small stat bonus does not guarantee another avoidance point at every value.
- The aggregate `saving_throw` field was dormant in generic resistance.
  Bless contributes one bounded 1..12-point threshold bonus from its named
  saving affect, after the original 15..85 base threshold clamp, still capped
  at 85. Duplicate/corrupt copies cannot multiply this bonus. Unrelated dormant
  equipment/class saving modifiers are **not** globally activated.
- Generic resistance now evaluates its original clamped formula without
  overflowing for extreme saved Wisdom or caller spell levels. Quotient/
  remainder arithmetic preserves the exact original result for valid values,
  including large values whose differences cancel; it does not clamp stats
  themselves or change the existing strict roll comparison.
- Different protective spells may coexist (+80 armor from all three), but
  recasting the same named effect neither stacks nor refreshes it. Existing
  class defenses are not replaced, amplified or given new immunity flags.
- Normal casting rejects duplicate, missing, cross-plane, invalid, dead,
  remote and self-only-mismatched targets before spending mana/recovery.
  Effect callbacks also validate room membership, planes, identity and
  modifier headroom. Item casts reuse this effect preflight; external item
  charge/consumption rules are unchanged.
- Legacy NPC avoidance/accuracy uses level rather than these player-style
  derived stat formulas. Storing an armor/hitroll affect on an NPC does not
  change that level-based path; NPC combat is not redesigned in this pass.

Giant Strength and Weaken remain empty and gated: Genesis's `APPLY_STR`
currently does not modify strength. Refresh/movement spells and class-overlapping
Frenzy/Darkblessing also remain gated. Do not advertise their presence in an
archive as a working Genesis implementation.

### Regression scope

The fifteenth C suite links the real engine and covers exact mana boundaries,
Wolfman doubling, normal recovery/pause/level/silence gates, target identity,
all three plane flags, all ten classes plus classless characters, duplicates,
item/NPC callbacks, null/remote arguments and bounded integer extremes.
It tests actual avoidance/accuracy threshold changes, deterministic real
saving rolls, separate-effect coexistence, affect expiry and named save/load
round-trips with reversal of modifiers. It does not mock the buff or resistance
implementation and does not use historical accounts.
Ten additional extreme-stat/level cases check the actual resistance rolls
with and without Bless, including large-value cancellation boundaries.

The catalog now also verifies all four historical slots/costs/targets and that
all 17 remaining empty spells are gated. Full multiplayer tuning and interactions
with every piece of equipment or privileged stat-reset command remain unverified.
