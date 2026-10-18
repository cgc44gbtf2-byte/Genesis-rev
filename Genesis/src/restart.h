#ifndef ANATHEMA_RESTART_H
#define ANATHEMA_RESTART_H

bool player_restart_checkpoint(void);
bool restart_arguments(int argc, char **argv, int *port, int *listener,
                       bool *copyover);
bool restart_listener_valid(int listener, int port);
bool copyover_recover(void);
void restart_copyover(CHAR_DATA *ch);

#ifdef RESTART_TESTING
/* Test-only fault injection; normal binaries have no hooks or bypasses. */
extern int restart_test_exec_errno, restart_test_exec_count;
extern int restart_test_manifest_fault;
extern void (*restart_test_exec_probe)(void);
#endif
#endif
