#ifndef ATOMIC_SAVE_H
#define ATOMIC_SAVE_H

#include <stdio.h>

/* A failed call never replaces the destination. UNCERTAIN means rename
 * succeeded but directory fsync failed: the new file is visible, but its
 * survival across power loss cannot be promised. Do not roll it back.
 * directory must be owned by this uid and not group/other writable; its
 * ancestors must also be trusted. No symlink directory or target is accepted.
 */
enum atomic_save_result {
    ATOMIC_SAVE_FAILED = -1,
    ATOMIC_SAVE_OK = 0,
    ATOMIC_SAVE_UNCERTAIN = 1
};

struct atomic_save_error {
    const char *stage;
    int number;
    int cleanup_number; /* Nonzero if a failed save left a temporary file. */
};

typedef int (*atomic_save_writer)(FILE *stream, void *context);
/* Fixed trusted basename only; does not relax player-name validation. */
enum atomic_save_result atomic_save_copyover(const char *directory,
    atomic_save_writer writer, void *context, struct atomic_save_error *error);

/* Name is an ASCII alphabetic player name, 1..255 bytes (not a path). */
int atomic_save_name_valid(const char *name);
enum atomic_save_result atomic_save(const char *directory, const char *name,
                                    atomic_save_writer writer, void *context,
                                    struct atomic_save_error *error);

#ifdef ATOMIC_SAVE_TESTING
/* Compiled out of production. A single injected failure per call. */
enum atomic_save_fault {
    SAVE_FAULT_NONE, SAVE_FAULT_CREATE, SAVE_FAULT_FLUSH, SAVE_FAULT_SYNC,
    SAVE_FAULT_CLOSE, SAVE_FAULT_RENAME, SAVE_FAULT_DIRSYNC,
    SAVE_FAULT_CLEANUP
};
extern enum atomic_save_fault atomic_save_test_fault;
#endif

#endif
