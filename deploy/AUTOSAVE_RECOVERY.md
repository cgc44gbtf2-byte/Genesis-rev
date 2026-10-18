# Switched-player autosave and save-failure recovery

This changes runtime save policy, not the player-file format, account password
algorithm, class abilities, power costs or hybrid availability.

## Confirmed failures and fixes

The new real-engine suite reproduced all five against the previous head:
periodic saves skipped switched originals; failed primary writes advanced
`save_time`; backup success also advanced it; manual save falsely said `Saved.`;
and a failed save during quit still discarded the loaded player/note.

`player_save.c` now separates runtime policy from serialization and atomic
replacement in `save.c`/`atomic_save.c`.

- Periodic selection resolves real switched originals, not controlled NPCs.
  Explicit `save`/autosave from a controlled NPC also saves its player.
- The transient `EXTRA_SWITCH` descriptor-ownership bit is not written to disk.
  Both native loaders clear it in older files too, preserving unrelated flags.
  Saving never clears the active in-memory switch or returns the descriptor.
  Additional red regressions confirmed both the serialized bit and stale
  reload state before this correction.
- `save_time` means last durably successful primary write. Backup success,
  failed/skipped writes and uncertain post-rename directory sync do not change
  it. A separate actual-attempt timestamp maintains fair selection.
- Primary and backup failures have separate pending flags, attempt times and
  backoff delays: 30, 60, 120, 240, then at most 300 seconds. A successful side
  cannot hide the failed side's deadline. A healthy destination can keep
  receiving ordinary online saves during the other's backoff.
- The periodic autosave budget remains one player per character-update call,
  with at most two destination writes. Oldest actual attempt wins among
  eligible players, so a broken destination cannot monopolize the selector.
  Backoff defines eligibility, not a guaranteed write deadline under load.
- Pending destinations retry while the player remains loaded, including after
  network disconnect. Ordinary non-pending link-dead autosave policy is not
  broadened. Switched originals need a descriptor lookup; regular connected
  players do not add a per-player descriptor scan.
- Clock rollback makes pending attempts eligible rather than freezing them.
  Deadline tests use elapsed time, not overflow-prone `time_t + delay`.
- Manual saves bypass background backoff deliberately. They distinguish full
  success, primary-only success, backup-only success, failure and ineligibility.
- A migrated-password pending flag clears only after successful durable primary
  persistence, never on backup-only, failed, uncertain or skipped writes.
  Migration hashing itself is unchanged.

## Logout and shutdown

An eligible player must first commit an ordinary authoritative primary snapshot
before quit consumes shaped equipment, discards a draft, closes the connection
or extracts the character. If it fails, quit is cancelled and loaded state is
retained for retry. Returning a switched descriptor to the original remains
intentional. Initial ineligible sleepers keep the legacy no-save quit policy.

The existing final normalized primary/backup save still runs during successful
quit, preserving its normal cleanup behavior. This adds a primary checkpoint to
the infrequent quit path. If a later final write fails after that checkpoint,
the checkpoint is the fallback and may reflect pre-cleanup transient state.
Backup retry is not promised after logout removes the character.

Idle auto-quit respects a pending primary's backoff. Graceful shutdown makes one
final forced attempt at both destinations and explicitly warns if a primary
save fails. Shutdown does not wait indefinitely for broken storage.

## Tests

`tests/test_autosave_recovery.c` uses the real engine, private Unix sockets,
temporary directories and the existing test-only atomic I/O fault hooks.
It covers switched periodic/manual saves without returning control, transient
switch-bit serialization and both native loaders of older files, metadata,
truthful outcomes, failed-quit preservation, independent directory failures,
native reload, fair one-player selection, capped backoff, disconnected retry,
credential-pending semantics, create/flush/fsync/close/rename/directory-sync
faults, authentication rejection and clock rollback.

The established serialization, atomic permissions/rename and authentication
suites remain in place. Test player data and credentials are synthetic.

## Limits

Retry metadata exists only in memory and is cleared when a character is
recreated/reloaded; there is no persistent queue or transaction journal.
The two files are independently atomic, not a two-file transaction. A
post-rename directory-sync failure is deliberately unacknowledged even if the
new file is visible.

Unwritable/full/broken storage, forced process termination and machine/power
failure can still lose changes that were never durably written. Final shutdown
failure warnings require operator action; this is not crash recovery or a
guarantee that shutdown always saves. Direct staff extraction and other legacy
destructive administrative paths are not transactionalized here.

Long multiplayer/economy soaks, real storage-fault/power-loss testing, body-part
provenance and class-cleared equipment/stat reconciliation remain separate.
