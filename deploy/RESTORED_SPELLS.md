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
21 empty legacy spell handlers stay unavailable through ordinary player
casting; their identifiers are not removed or reassigned.
