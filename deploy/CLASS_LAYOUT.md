# Class-code ownership and cleanup stages

## Stage 1: source layout, without gameplay changes

The ten existing classes now have owned directories under
`Genesis/src/classes/`. Fourteen C files and Mage's header were moved from the
engine directory **without changing their contents**. Class identities,
power costs, ranks, targeting, flags, save fields, skill slots and disabled
abilities are not changed by this reorganization. Hybrids remain unavailable.

The move was checked against the Git blob IDs at pre-cleanup head
`4ad40e92ffb83733519db718dbc985b0abaac971`: all 428,364 bytes in the 15 files
match. Git can display these changes as 100% renames rather than new powers.
Diagnostic/debug source paths naturally reflect the new directories.

## Stage 2: bounded extraction from shared `clan.c`

Six class-owned handlers have been extracted into three additional class
modules. Their function bodies were compared byte-for-byte with
pre-extraction head `d655535ce3d93a2439ff4107309ed02b5afa2c95`, including
messages, random calls, checks and state mutations. The remaining `clan.c`
diff contains only their removal and blank lines.

| New owner | Existing functions moved |
| --- | --- |
| `vampire/presence.c` | `do_majesty`, `do_awe` |
| `vampire/beast.c` | `do_humanity`, `do_beastlike` |
| `werewolf/instincts.c` | `do_shadowsight` (Umbral Vision), `do_rage` |

Public symbols, command-table order, aliases and indirect call sites are
unchanged. Humanity and Beastlike remain existing internal callbacks; they
were not added as player commands. Vampire Beast callbacks still call the
shared fangs/claws/sight helpers, and Rage still calls the existing Werewolf
transformation code.

Fangs, Claws, Nightsight, Calm and Flex remain in `clan.c`. In particular,
Calm explicitly supports both Werewolves and Ninjas. At this stage Earthmeld
and the Werewolf transformation helpers remained for a later bounded review;
location in a shared file does not mean every class can use them.

No cooldown, eligibility, random distribution, resource payment, damage,
serialized field, class predicate or disabled ability is changed. The stored
Majesty cooldown remains 120, Presence ranks remain 1/5 for Awe/Majesty, and
Rage retains its 10–20 increase, transformation threshold and 12-pulse wait.
Existing legacy wording and edge cases are not silently corrected during
this extraction.

## Stage 3: Vampire states, targeting and illusion ownership

Seven more handlers have byte-identical bodies relative to pre-extraction head
`0f6598af956132dce3ec8070311eabcdfbc90a34`. The rest of `clan.c` differs only by
their removal and blank lines. The existing copyright notices are retained.
This describes the mechanical extraction commit; the separate Possession
hardening commit below intentionally adds one ownership guard.

| Owner | Existing handlers moved |
| --- | --- |
| `vampire/state.c` | `do_regenerate`, `do_earthmeld` |
| `vampire/obfuscate.c` | `do_vanish` |
| `vampire/presence.c` | `do_summoning` |
| `vampire/dominate.c` | `do_possession` |
| `vampire/chimerstry.c` | `do_mirrorimage`, `do_control` |

Regenerate keeps its Vampire-only, **no-discipline-rank** gate. Earthmeld keeps
both existing conditions: `get_disc()` returns zero for non-Vampires, so its
effective class/rank gate remains Vampire/Protean 3. No condition was simplified.
Vanish keeps Obfuscate 1/self and 3/group behavior; Summoning keeps Presence 4;
Possession keeps Dominate 3; Mirror Image/Control keep Chimerstry 1/5.
Public names, registration order, wait states and resource behavior are unchanged.

`do_vampire` is a general feeding/embracing-consent toggle, not a Vampire-only
power. It stays shared, as do Shadowplane, Shield, Scry, Truesight and the
multi-class illusion helpers. Serpent and the other transformations remain
for a separate lifecycle review: `clear_stats()` can trigger autosaving and
must not be treated as a harmless local flag reset.

### Separate hardening: reciprocal familiar ownership

The mechanical stage exposed an existing Possession ownership bug: unlike
Control Illusion, it allowed an already-bound caster to retarget. A synthetic
real-engine probe reproduced the first NPC retaining its `wizard` pointer
after the caster's `familiar` pointer changed to a second NPC.

A separate commit rejects Possession when `pcdata->familiar` is non-null,
preserving the current pair rather than silently dropping or destroying the
first familiar. It does not add new costs or change rank/level thresholds.
The regression failed against the original implementation, then passed with
the guard. Tests cover ranks 3–7, both Possession/Control Illusion ownership
directions and successful rebinding once NPC extraction clears the first pair.
This prevents the reproduced orphan binding; it is not a claim that every
familiar/session/logout lifecycle edge case has been audited.

### Current class-source ownership

| Owning class | Files under `Genesis/src/classes/` | Responsibility |
| --- | --- | --- |
| Vampire | `vampire/disc.c`, `vampire/vampire.c`, `vampire/presence.c`, `vampire/beast.c`, `vampire/state.c`, `vampire/obfuscate.c`, `vampire/dominate.c`, `vampire/chimerstry.c` | Disciplines, named teaching/blood powers, Presence, Beast, recovery/melding, invisibility, possession and illusions |
| Werewolf | `werewolf/werewolf.c`, `werewolf/wwbreed.c`, `werewolf/wwauspice.c`, `werewolf/wwtribe.c`, `werewolf/instincts.c` | Progression/menu, breed/auspice/tribe gifts, Umbral Vision and Rage |
| Sorcerer | `sorcerer/sorcerer.c` | Spellbook creation, pages, chanting and associated effects |
| Drow | `drow/drow.c` | Drow powers, ranks and attunement |
| Highlander | `highlander/highland.c` | Existing Highlander training, attacks and forging |
| Mage | `mage/mage.c`, `mage/mage.h` | Sphere powers, progression and Mage constants |
| Dragon | `dragon/dragon.c` | Dragon form, breath, age/skill progression and physical abilities |
| Ninja | `ninja/ninja.c` | Principles, training and Ninja powers |
| Monk | `monk/monk.c` | Monk training, martial powers and Stun Fist |
| Elemental | `elemental/elemental.c` | Elemental progression, attacks, support and forms |

Module ownership is not an exclusive permission rule: an existing command may
deliberately serve more than one class. Those checks were not widened or
restricted. Mage's header remains transitively available through `merc.h` for
compatibility; it is not yet an isolated public/private API boundary.

### One authoritative class-source manifest

`classes/modules.mk` explicitly selects the 21 class C files for **both** the
production executable and real-engine regression fixtures. A file copied into
a class directory is not silently compiled into the game.

Both builds retain the engine-root include path, including when callers
provide their own preprocessor flags. Nested test objects are created under
`tests/stun_engine/classes/`; normal and sanitizer test artifacts remain
separate from production objects. Production output stays at `Genesis/src/merc`,
so the service and install paths do not change.
The clean targets also remove the former flat-layout class objects and
dependency files left by an in-place branch update.

`classes/class_ids.h` owns the serialized class constants. The values are
unchanged: None 0, Vampire 1, Werewolf 2, Sorcerer 4, Drow 8, Highlander 16,
Mage 32, Dragon 64, Ninja 128, Monk 256 and Elemental 512.
They remain exact identities, **not** combinable parent/ancestry masks.

## Shared code deliberately not moved wholesale

These files mix concerns or own engine-wide lifecycle rules. Moving them into
one class directory would imply an ownership boundary that does not exist.

| Engine file | Class-related responsibility still present |
| --- | --- |
| `clan.c` | Large legacy Vampire collection, shared fangs/claws/sight/form commands, class/clan/tribe training and helpers |
| `more.c` | Additional legacy commands and class-related utilities |
| `clearclass.c` | Cross-class teardown/reset, including shared fields and flags |
| `interp.c` | Command registration, permission/recovery checks and shared command dispatch |
| `fight.c`, `act_info.c` | Class-dependent damage, avoidance and derived-stat behavior |
| `update.c` | Per-tick effects, class upkeep and expiry |
| `comm.c` | Session/authentication lifecycle and character initialization |
| `save.c`, `atomic_save.c` | Player records and durable checked-save behavior |
| `merc.h` | Shared character structures and legacy command declarations |
| `magic.c`, `const.c` | Generic spell implementation and stable spell/skill registration |

For example, `do_fangs`, `do_claws` and `do_nightsight` in `clan.c` already
serve several classes. They must not be relabelled Vampire-only during a move.

## Known data coupling: separate migration required

The existing player format reuses fields for mutually exclusive class
semantics. A confirmed example is `pcdata->cland[0..2]`: Vampire clan discipline
choices and Mage favored-sphere choices use these same slots.

Stage 1 does **not** rename, duplicate or reinterpret those saved fields.
Separating them requires independently tested compatibility accessors and/or
an explicit versioned save migration. It must not be bundled into file moves
or silently activated by adding another class bit.

Class flags, resource pools, temporary transformations and teardown paths need
the same ownership review before hybrid work. Raw donor IDs and overloaded
arrays are not a compatibility interface.

## Regression and review contract

The sixteenth C suite, `tests/test_class_identity.c`, pins every saved class
number at compile time and checks all ten identity predicates for PCs, NPCs,
classless characters, unsupported composite values and integer extremes.

The seventeenth C suite, `tests/test_class_handlers.c`, first passed against
the original `clan.c` under ASan/UBSan before the extraction. The same real-engine
fixture then characterizes the moved handlers and retained shared abilities.
It checks Presence ranks/toggles/cooldown boundaries, seeded Beast progression
and Golconda, Protean helper calls, the actual `awe`/`majesty`/`umbralvision`/`rage`
command routes, Rage transformation/Ahroun cap, resource and wait behavior,
all ten classes plus classless actors, NPCs without player data, shared
fangs/claws/sight eligibility, Flex and Ninja/Werewolf Calm.
It uses synthetic state only and never loads accounts or a world archive.

The eighteenth suite, `tests/test_vampire_modules.c`, passed under ASan/UBSan
against the pre-extraction implementation before running against the owned
modules. It exercises all seven real command routes, rank and class rejection
(including NPCs without `pcdata`), regeneration/melding toggles, group invisibility
target isolation, summoning consent/fighting/room restrictions and NPC flags,
possession/control guards and bindings, actual mirror creation and the existing
five-follower cap, unchanged resources and waits. Clone gear-copy branches and
production world configuration are not covered by its synthetic no-gear clones.

Ten Python layout checks cover:

- All ten class owners and all 21 explicitly selected source files.
- No stale root-level class copies or removed Mage header.
- Exact serialized class constants and unchanged identity predicates.
- Equality of the actual production/test Makefile source expansions.
- Owned header wiring and the shared module manifest.
- Single public class-command definitions and updated fixture includes.
- Cleanup coverage for obsolete flat-layout build artifacts.
- Exact extracted-handler ownership and retained shared-handler locations.

`make test` runs these layout checks automatically. Existing catalog checks
now inspect the same manifest-selected class sources instead of only root
`*.c` files; command, world-reference and skill-reference coverage is retained.

Verification now uses all **20 normal and ASan/UBSan C suites**,
**20 unique Python catalog/layout checks**, the full production build and
isolated TCP/TERM smoke test, followed by the Debian 12/13 and Ubuntu
22.04/24.04 CI matrix. A test result is recorded in the PR only after it passes;
this document specifies the test scope, not a claim of a pending run's success.

The nineteenth suite, `tests/test_damage_messages.c`, covers the separate
Gathering-wording change without expanding class-cleanup scope: unchanged
Genesis thresholds, six damage types, recipient/number visibility, unchanged
actor state and bounded handling of long/missing attack names.

The twentieth suite, `tests/test_transformations.c`, covers the shared
transformation persistence boundary. Cleanup now lives in the non-saving
`transformation.c:reset_form_stats()` helper. All nine known transformation
reset sites defer autosaving until each command finishes its form state;
`clear_stats()` keeps its reset-and-save compatibility contract. Coverage
includes six forms in both directions, equipment/affect cleanup, Dragon
features, class/NPC rejection, old-file retention on pre-rename failure,
uncertain directory fsync, independent primary/backup failures and retries.
This is a lifecycle bug fix, not another handler extraction: class module
and extracted-handler counts remain 21 and 13 respectively. The player-file
format and class balance do not change. Follower accounting, overlapping
temporary-flag ownership and complete class teardown still require work.

These checks do not prove complete multiplayer balance, all legacy behavior,
power-cut recovery or every production systemd interaction. The preserved
source bytes provide additional review evidence, not an excuse to skip tests.

## Next stages

1. Extract clearly class-owned handlers from shared legacy files in bounded
   changes, preserving public command symbols and shared helpers.
2. Separate class-facing interfaces from the global header and introduce
   semantic accessors for overloaded progression/resource fields.
3. Design and verify any versioned player-data migration separately, with
   old-save loading, malformed records, save failure and rollback tests.
4. Consolidate genuinely identical validation/payment/recovery code only
   after real-engine tests pin each class's existing differences.
5. Profile combat/update paths before runtime optimization. No speedup or
   allocation reduction is claimed from moving source files.
6. Consider an explicitly gated hybrid profile only after the relevant parent
   data and lifecycle boundaries are safe.

Legacy warnings and shared-code coupling remain visible work, not hidden
behind a claim that class modularization is finished. Main, deployment state
and game balance are outside this source-layout change.
