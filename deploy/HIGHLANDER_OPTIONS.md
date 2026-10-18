# Highlander: verified safety work and source comparisons

The current Highlander has four six-rank training tracks (Attack, Defense,
Weaponry, Powers). It has Heighten, Bladeflash, Bash, Htwist, Judge, Lunge
and a 50-primal/100-quest-point katana forge. None of this patch changes a
rank threshold, a charge, a passive combat bonus or player-save layout.

## Safety changes in this branch

- Bladeflash is a hostile action. It now rejects self/incapacitated targets,
  checks the existing combat-safety and combat-entry paths before removing
  Heighten or adding blindness, and does not engage if the blind skill is
  unavailable. It does not require a sword; a blade-only requirement would
  change the existing ability rules and is **not** introduced here.
- Reforging an owned ordinary weapon still costs 50 primal and 100 quest
  points, copies its four weapon values and makes a Highlander katana.
  Purchased numeric quest enhancements, allowed enchantment flags, the
  already-spent quest-point tally, and runtime object affects now move once
  to the new weapon; the maker and owner strings retain their provenance.
  There is no refund or duplication of those points.
- Temporary, vanishing, class-restricted, special-action, custom-power,
  custom-description, or otherwise incompatible weapons are rejected
  **before** the charge and before the source is destroyed. Artifact/relic
  sources remain prohibited. The new katana retains its native Highlander
  restrictions and aggravated status. No global item-copy routine or save
  format is changed. Such rejected weapons remain available to their owner;
  lossless transformation of arbitrary special objects is not promised.
- Training, damage and Judge output now use bounded formatting. A native
  Judge regression examines an NPC whose short description exceeds the
  command's local message buffer.

The full-engine Bladeflash regression covers self, Peace, link-dead, combat
engagement and target blindness. The full-engine forge regression covers
protected failures, allowed investment/affect transfer, unique ownership,
and a real object save/load. The existing ordinary forge case remains.

## Other attached Highlander implementations: concepts, not copied code

| Source | Implemented Highlander abilities | Fit for Anathema |
|---|---|---|
| Gathering 1.01 (`class_highlander.c`) | Seven described ranks: Truesight, Truesense, Strength, Oneness, Forge, Disarm, Agility; a mercenary hire/fire lifecycle and passive regeneration | Truesight, Strength and Forge overlap. Truesense could be an informational command; hiring requires a separate consent/leadership design. Its regeneration path includes brittle legacy arithmetic; do not port directly. Listed upper ranks are explicitly “Under construction.” |
| GWLow5 (`interp.c`, `clan.c`) | Class recruitment/leadership and a conjured katana | Clan governance already exists in Anathema; the conjured weapon is less protective of provenance than Anathema's forge. No new, self-contained sword combat track found. |
| TSOB (`highlander.c`, `lander.c`, `fight.c`, `cor_fight.c`, `cor_update.c`) | Katana, Awareness, aura inspection, Counter, Chaos Shield, Timewalk and a 22-flag Quickening progression; verified native hooks for strength, speed, resistance, parry/dodge, Lightning Strike, kick variants and regeneration | Counter and Lightning Strike are strong swordmaster concepts; sensing is lower risk. Its saved Quickening bits, damage reactions and stat scale need independent redesign. A flag grant or menu entry is not proof every advertised power works. `swordthrust` lives in Kensai and checks `IS_KENSAI`, not Highlander, even though some shared kick hooks read Quickening flags. |
| Legend of Chrystancia (`highlander.c`) | Out-of-combat healing, Blade Spin, elemental/soul swordcraft, sensing Quickening, Ancestral Path, several physical sword techniques and Quickening modes | Richest candidate pool, but tightly coupled to a different saved Quickening system, weapon caps, death/teleport routines and arena rules. Do not copy its direct-damage multipliers, instant behead, travel or resource spending verbatim. |
| AtD 2.0 (`low_wiz.c`, `clan.c`) | A conjured katana and shared Truesight, Scry, Read Aura and Shield access | Some Highlander-specific rank checks are commented out. Information abilities can inform a new design, but the source does not provide a sound ready-to-import progression. |
| Godwars 1996/2k6 and Ethereal Mists | Shared Truesight access and generic Highlander combat/class handling, but no comparable standalone ability track found in these inventories | Heighten already overlaps the meaningful sight feature. No new class powers to silently mark “missing” from Genesis. |
| Myth 3, Vortex and VW 1.0.2 | No Highlander-specific ability implementation found in the supplied source inventories | Do not mistake similarly named generic “quicken” commands or other classes' weapon abilities for Highlander powers. |

This is a source inventory, not a claim that every unrelated generic weapon
command in every archive was audited. The older distributions contain
copyright/license notices; new features should be independently designed and
implemented, retaining the current project's notices.

### Adaptation candidates

The separate original [Counter implementation](HIGHLANDER_COUNTER.md) now
implements the first concept below as a one-swing guard with a deferred
native riposte. The remaining candidates are not implemented.

| Priority | Concept | Adaptation boundary |
|---|---|---|
| 1 | TSOB Counter | An equipped-sword defense/reply, bounded chance or duration, with no recursive counter chains. |
| 2 | TSOB Lightning Strike | At most one capped follow-up from a successful qualifying weapon strike; check target survival and prevent proc-on-proc loops. |
| 3 | LoC Blade Spin or Moon Slash | A finite active multi-strike maneuver with explicit cost and recovery; revalidate the target after each native damage call. |
| 4 | TSOB/AtD aura inspection or Gathering-style sensing | Information only, respecting visibility and privacy; do not expose global locations automatically. |
| 5 | LoC Heal Self | Out-of-combat, capped recovery using the current mana model, not its skill-sum multiplication or a new Quickening pool. |

Start with a **guarded sword technique**, inspired by a TSOB-style Counter
or a LoC-style Blade Spin/Moon Slash but written for Anathema's existing
weapon slots, native damage cap, combat-safety checks and target-liveness
rules. A counter must not recursively trigger damage reactions. Require an
equipped appropriate sword and a learned Powers rank; set a finite mana cost
and recovery lag after playtesting. This creates a clearer swordmaster
decision without a new saved resource field. An informational aura/Quickening
sense would be lower risk but would not address the lack of active sword
options. Healing and a persistent Quickening economy need separate balance
and save-migration designs; instant executions should not be imported.

The prior 50-fight benchmark and the ten-fight Highlander tactics follow-up
remain exploratory. No ability port, new resource or balance tuning is
included in this safety patch.
