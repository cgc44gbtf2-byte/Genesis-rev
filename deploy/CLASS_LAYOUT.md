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

| Owning class | Files under `Genesis/src/classes/` | Responsibility |
| --- | --- | --- |
| Vampire | `vampire/disc.c`, `vampire/vampire.c` | Additional discipline powers; named teaching and guarded blood powers |
| Werewolf | `werewolf/werewolf.c`, `werewolf/wwbreed.c`, `werewolf/wwauspice.c`, `werewolf/wwtribe.c` | Progression/menu, breed, auspice and tribe gifts |
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

`classes/modules.mk` explicitly selects the 14 class C files for **both** the
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

Nine new Python layout checks cover:

- All ten class owners and all 14 explicitly selected source files.
- No stale root-level class copies or removed Mage header.
- Exact serialized class constants and unchanged identity predicates.
- Equality of the actual production/test Makefile source expansions.
- Owned header wiring and the shared module manifest.
- Single public class-command definitions and updated fixture includes.
- Cleanup coverage for obsolete flat-layout build artifacts.

`make test` runs these layout checks automatically. Existing catalog checks
now inspect the same manifest-selected class sources instead of only root
`*.c` files; command, world-reference and skill-reference coverage is retained.

Verification for this stage uses all **16 normal and ASan/UBSan C suites**,
**19 unique Python catalog/layout checks**, the full production build and
isolated TCP/TERM smoke test, followed by the Debian 12/13 and Ubuntu
22.04/24.04 CI matrix. A test result is recorded in the PR only after it passes;
this document specifies the test scope, not a claim of a pending run's success.

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
