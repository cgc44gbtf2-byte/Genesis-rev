# Runtime transformation and ownership safety

This pass hardens transient links; it does not introduce hybrids, add powers,
change resource costs or serialized fields, or change normal form statistics.
Class implementations remain in their existing modules. Shared extraction and
descriptor ownership stay in the legacy engine files for this bounded change.

## Confirmed regressions

Before the fix, the new real-engine tests reproduced:

- Extracting an obsolete form erased the player's different current form.
- Extracting an obsolete familiar erased the owner's different current familiar.
- Removing a switched original left its descriptor pointing at the recycled
  player, even though deleting the controlled NPC already returned correctly.
- Destroying a valid severed head with the altar unavailable removed the
  player from its room and failed to place it in another.
- Staff class reset dropped the player-side familiar pointer without releasing
  the NPC's owner link.

Inspection also found active switched input reset only the NPC's idle timer.
The original player could consequently reach idle auto-quit while using it.
An additional real-engine note-editor quit regression reproduced a double free:
the silent socket-close path freed the in-world player, and quit then freed it
again, making the recycled character/player-data lists self-referential.
The player-save guard also rejected note and world-editor session states.

## Guarantees and preserved behavior

- Object extraction resolves its character pointer against the live character
  list and checks reciprocity before modifying player state. A stale link
  cannot clear a newer form, including when character storage is recycled.
- Player extraction and class reset clear inbound object, wizard, embrace and
  proposal references. No foreign object or familiar is modified merely because
  an obsolete outward pointer names it.
- Only the current, live, reciprocal NPC familiar is retired on full owner
  extraction, preserving the previous valid logout policy. Other NPCs are not
  destroyed because of a stale pointer. Stray owner links are detached.
- Class reset releases NPC control but leaves the NPC alive, as before. It also
  returns switched players to their own descriptor before resetting flags.
  Existing form objects are detached, not deleted or repriced by class reset.
- Switched originals regain descriptor ownership before quit/save. Full player
  removal closes descriptors that name the player, with references detached
  before closing to avoid a nested character free. NPC-body extraction still
  returns the player rather than closing the player's connection.
- Closed descriptor structures no longer retain character/original pointers.
  Input and successful takeover reset both the controlled and original idle
  timers. No idle threshold or ordinary idle-quit policy is changed.
- Both socket-close paths recognize note editors as in-world sessions and leave
  character disposal to quit/extraction. Discarded in-progress notes are cleared.
  Save eligibility and the existing shutdown selector share an explicit
  playing/note/world-editor classification; login shells remain excluded.
  The ordinary oldest-player autosave selection now includes these editors.
- Valid head destruction still clears head/polymorph state, sets one hit point
  and resting position, and uses the altar when present. When absent, the
  current valid room is retained and a staff diagnostic is emitted.
- Soft extraction (`fPull == FALSE`) still retains the living player's familiar;
  it only detaches a canonical object link if that object actually points back.

## Regression coverage

`tests/test_lifecycle_ownership.c` links the actual engine. It uses temporary
player/backup directories, synthetic prototypes/rooms and private Unix socket
pairs; it never loads or changes live player data.

It covers stale versus current forms, foreign object ownership, stray object
backlinks on removal, reciprocal/nonreciprocal familiar deletion, orphan owner
references, controlled-body deletion, full switched-original removal, switched
idle-quit, missing-altar head recovery and switched staff class reset.
`tests/test_comm.c` covers both idle timers, empty descriptors, ordinary player
activity and successful switched takeover activity.
The ownership suite also quits and reloads from all five note-editor stages,
checks saved gold and acyclic recycled lists. `tests/test_save.c` covers primary
and backup saves in all five note stages and four world editors without
changing editor state; login/NPC/low-level rejection tests remain.

## Remaining work

This is not a complete certification of every form, death, save or multiplayer
interaction. Existing part-name/value provenance is unchanged. Persistent
identity-based ingredient validation, crash/restart and save-failure recovery,
automatic retry policy, prolonged multiplayer stress, world-specific scripting
and live service/power-loss tests remain separate work.
Periodic save scheduling while switched and automatic save-retry/reporting
policy are not redesigned here.

Recovery with no valid current room and no altar is not solved by retaining a
current room. The existing roomless-character extraction guard remains.
Cleared-class equipment eligibility/stat reconciliation and familiar inventory
policy are not redesigned. The live-list checks add bounded linear work only
on ownership cleanup, not a new per-tick world scan; no measured performance
improvement is claimed.
Ordinary NPC extraction does not acquire an additional world-object scan.
