#include <time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "merc.h"

extern	int	control, port;

bool	write_to_descriptor	args( ( long desc, char *txt, long length ) );
void	attempt_copyover( void );

/* Only the normal game loop may inspect players or save/close anything.
 * Repeated stop requests never force an unsafe save.
 */
volatile sig_atomic_t shutdown_requested = 0;

static void request_shutdown( int sig )
{
    shutdown_requested = 1;
}

static void install_signal( int sig, void (*handler)(int) )
{
    struct sigaction action;

    memset( &action, 0, sizeof(action) );
    action.sa_handler = handler;
    sigemptyset( &action.sa_mask );
    /* No SA_RESTART: a stop should interrupt the game's select sleep. */
    if ( sigaction( sig, &action, NULL ) < 0 )
    {
        perror( "init_signals: sigaction" );
        exit( 1 );
    }
}

void init_signals( void )
{
    install_signal( SIGTERM, request_shutdown );
    install_signal( SIGINT, request_shutdown );
    install_signal( SIGPIPE, SIG_IGN );

    /* Corrupted state cannot safely be saved or copied over. Fatal signals
     * keep OS termination/core-dump semantics. SIGHUP/SIGIO are not graceful
     * stop requests; SIGPFILE (77) is not an OS signal. No gameplay, stdio,
     * allocator or exit runs in a handler.
     */
    install_signal( SIGBUS, SIG_DFL );
    install_signal( SIGSEGV, SIG_DFL );
    install_signal( SIGILL, SIG_DFL );
    install_signal( SIGFPE, SIG_DFL );
    install_signal( SIGTRAP, SIG_DFL );
    install_signal( SIGABRT, SIG_DFL );
}

void attempt_copyover( void )
{
    DESCRIPTOR_DATA *d;
    FILE *fp;
    char buf[50], buf2[50];

    fp = fopen (COPYOVER_FILE, "w");
    if (!fp) return;

    log_string("CRASHING..");
    for (d = descriptor_list; d; d = d->next)
    {
	CHAR_DATA *ch = d->original ? d->original : d->character;
	save_char_obj(ch);
	if (!ch)
	{
	    write_to_descriptor(d->descriptor,"We are crashing. Please reconnect.\n\r", 0);
	    close_socket(d);
	    continue;
	}
	fprintf(fp, "%li %s %s\n", d->descriptor, d->character->name, d->host);
	write_to_descriptor(d->descriptor, "GENESIS IS ATTEMPTING TO NOT CRASH.  OH THE HUMANITY.\n\r", 0);
    }

    fprintf(fp, "-1\n");
    fflush(fp);
    fclose(fp);

    sprintf(buf, "%d", port);
    sprintf(buf2, "%d", control); 

    execl(EXE_FILE, "merc", buf, "copyover", buf2, (char *) NULL);

    for (d = descriptor_list; d; d = d->next)
	write_to_descriptor(d->descriptor, "COPYOVER FAILED! Attempting NORMAL boot!\n\r", 0);

    return;
}
