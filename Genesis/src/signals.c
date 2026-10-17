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

bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
void	attempt_copyover( void );
void	crash_notice( int );

void init_signals( void )
{
    signal(SIGPFILE,	crash_notice);
    signal(SIGFPE,	crash_notice);
    signal(SIGINT,	crash_notice);
    signal(SIGHUP,	crash_notice);
    signal(SIGSEGV,	crash_notice);
    signal(SIGTERM,	crash_notice);
    signal(SIGIO,	crash_notice);
    signal(SIGPIPE,	crash_notice);
    signal(SIGTRAP,	crash_notice);
    return;
}


void crash_notice( int sig )
{
    DESCRIPTOR_DATA *d;
    DESCRIPTOR_DATA *d_next;

    for (d = descriptor_list; d; d = d->next)
	if (d->character && d->connected <= CON_PLAYING) do_wake(d->character, "");
    for (d = descriptor_list; d; d = d->next)
	if (d->character && d->connected <= CON_PLAYING) do_call(d->character, "all");
    for (d = descriptor_list; d; d = d->next)
	if (d->character && d->connected <= CON_PLAYING) do_save(d->character, "");

    for (d = descriptor_list; d; d = d_next)
    {
	d_next = d->next;

	if (d->character)
	    stc("GW: Genesis is about to crash. Hold on tight.. \n\r", d->character);
	else
	    write_to_descriptor(d->descriptor, "We are about to crash. Hold on..", 0);

	close_socket(d);
    }
    abort();
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
