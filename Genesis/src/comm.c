  /***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  God Wars Mud improvements copyright (C) 1994, 1995, 1996 by Richard    *
 *  Woolcock.  This mud is NOT to be copied in whole or in part, or to be  *
 *  run without the permission of Richard Woolcock.  Nobody else has       *
 *  permission to authorise the use of this code.                          *
 ***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <malloc.h>
#include <crypt.h>

#include "merc.h"


/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( long  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>	/* for RLIMIT_NOFILE */
extern volatile sig_atomic_t shutdown_requested;
#endif

#if defined(apollo)
#undef __attribute
#endif

/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( long s, struct sockaddr *addr, long *addrlen ) );
int	bind		args( ( long s, struct sockaddr *name, long namelen ) );
void	bzero		args( ( char *b, long length ) );
int	getpeername	args( ( long s, struct sockaddr *name, long *namelen ) );
int	getsockname	args( ( long s, struct sockaddr *name, long *namelen ) );
long	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
long	listen		args( ( long s, long backlog ) );
long	setsockopt	args( ( long s, long level, long optname, void *optval,
			    long optlen ) );
long	socket		args( ( long domain, long type, long protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, long length ) );
#endif

#if	defined(__hpux)
long	accept		args( ( long s, void *addr, long *addrlen ) );
long	bind		args( ( long s, const void *addr, long addrlen ) );
void	bzero		args( ( char *b, long length ) );
long	getpeername	args( ( long s, void *addr, long *addrlen ) );
int	getsockname	args( ( long s, void *name, long *addrlen ) );
long	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
long	listen		args( ( long s, long backlog ) );
long	setsockopt	args( ( long s, long level, long optname,
 				const void *optval, long optlen ) );
long	socket		args( ( long domain, long type, long protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fcntl.h>
#endif

#if	defined(linux)
#if 0
long	accept		args( ( long s, struct sockaddr *addr, long *addrlen ) );
long	bind		args( ( long s, struct sockaddr *name, long namelen ) );
#endif
//long	close		args( ( long fd ) );
// long	getpeername	args( ( long s, struct sockaddr *name, long *namelen ) );
// long	getsockname	args( ( long s, struct sockaddr *name, long *namelen ) );
// long	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
// long	listen		args( ( long s, long backlog ) );
/* kavirpo: put this in for nin site.
long	read		args( ( long fd, char *buf, long nbyte ) );
*/
//long	select		args( ( long width, fd_set *readfds, fd_set *writefds,
//			    fd_set *exceptfds, struct timeval *timeout ) );
//long	socket		args( ( long domain, long type, long protocol ) );
/* kavirpoint: put this in for nin site.
long	write		args( ( long fd, char *buf, long nbyte ) );
*/
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

long	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	long		errno;
#endif

#if	defined(MSDOS)
long	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
long	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
long	close		args( ( long fd ) );
long	fcntl		args( ( long fd, long cmd, long arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
long	read		args( ( long fd, char *buf, long nbyte ) );
long	select		args( ( long width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
long	write		args( ( long fd, char *buf, long nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( long s, struct sockaddr *addr, long *addrlen ) );
long	bind		args( ( long s, struct sockaddr *name, long namelen ) );
long	close		args( ( long fd ) );
long	fcntl		args( ( long fd, long cmd, long arg ) );
long	getpeername	args( ( long s, struct sockaddr *name, long *namelen ) );
int	getsockname	args( ( long s, struct sockaddr *name, long *namelen ) );
long	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
long	listen		args( ( long s, long backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
long	read		args( ( long fd, char *buf, long nbyte ) );
long	select		args( ( long width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
long	setsockopt	args( ( long s, long level, long optname, caddr_t optval,
			    long optlen ) );
long	socket		args( ( long domain, long type, long protocol ) );
long	write		args( ( long fd, char *buf, long nbyte ) );
#endif

/*
 * This includes Solaris SYSV as well.
 */
#if defined(sun)
long	accept		args( ( long s, struct sockaddr *addr, long *addrlen ) );
long	bind		args( ( long s, struct sockaddr *name, long namelen ) );
void	bzero		args( ( char *b, long length ) );
long	close		args( ( long fd ) );
long	getpeername	args( ( long s, struct sockaddr *name, long *namelen ) );
int	getsockname	args( ( long s, struct sockaddr *name, long *namelen ) );
long	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
long	listen		args( ( long s, long backlog ) );
long	read		args( ( long fd, char *buf, long nbyte ) );
long	select		args( ( long width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
long	setsockopt	args( ( long s, long level, long optname, 
                            const char *optval, long optlen ) );
#else 
long	setsockopt	args( ( long s, long level, long optname, void *optval,
			    long optlen ) );
#endif
long	socket		args( ( long domain, long type, long protocol ) );
long	write		args( ( long fd, char *buf, long nbyte ) );
#endif

#if defined(ultrix)
long	accept		args( ( long s, struct sockaddr *addr, long *addrlen ) );
long	bind		args( ( long s, struct sockaddr *name, long namelen ) );
void	bzero		args( ( char *b, long length ) );
long	close		args( ( long fd ) );
long	getpeername	args( ( long s, struct sockaddr *name, long *namelen ) );
int	getsockname	args( ( long s, struct sockaddr *name, long *namelen ) );
long	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
long	listen		args( ( long s, long backlog ) );
long	read		args( ( long fd, char *buf, long nbyte ) );
long	select		args( ( long width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
long	setsockopt	args( ( long s, long level, long optname, void *optval,
			    long optlen ) );
long	socket		args( ( long domain, long type, long protocol ) );
long	write		args( ( long fd, char *buf, long nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_free;	/* Free list for descriptors	*/
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newbielock;		/* No newbies allowed		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* Time of this pulse		*/
long		    chars_logged_in;	/* Stores the types of char	*/
long		    total_number_login;	/* Stores the total logins	*/
long		    high_number_login;	/* Stores most logins at once	*/
char		    first_place[40];	/* Name of best score on	*/
char		    second_place[40];	/* Name of 2nd best score on	*/
char		    third_place[40];	/* Name of 3rd best score on	*/
long		    score_1 = 0;	/* Actual scores		*/
long		    score_2 = 0;	/* Actual scores		*/
long		    score_3 = 0;	/* Actual scores		*/
long		    world_affects = 0;	/* World Affect bits		*/
long		    chainspell = 0;	/* Used for mage chain spells	*/
extern              BAN_DATA          *     ban_list;



/* Colour scale char list - Calamar */

char *scale[SCALE_COLS] = {
	L_RED,
	L_BLUE,
	L_GREEN,
	YELLOW
};

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( long desc, char *txt, long length ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( long desc, char *txt, long length ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_kickoff		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void	bust_a_prompt		args( ( DESCRIPTOR_DATA *d ) );



/* Needs to be global because of do_copyover */
int port, control;

int main( int argc, char **argv )
{
    struct timeval now_time;
    bool fCopyOver = FALSE;

    init_signals();

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

#ifdef RLIMIT_NOFILE
#ifndef min
# define min(a,b)     (((a) < (b)) ? (a) : (b))
#endif
        { 
        struct  rlimit rlp;
        (void)getrlimit(RLIMIT_NOFILE, &rlp);
        rlp.rlim_cur=min(rlp.rlim_max,FD_SETSIZE);
        (void)setrlimit(RLIMIT_NOFILE, &rlp);
	}
#endif

    /*
     * Init time and encryption.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );
    strcpy( first_place, "Nobody" );
    strcpy( second_place, "Nobody" );
    strcpy( third_place, "Nobody" );
    wizlock = FALSE;
    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 4000;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
        /* Are we recovering from a copyover? */
        if (argv[2] && argv[2][0])
        {
                fCopyOver = TRUE;
                control = atoi(argv[3]);
        }
        else
                fCopyOver = FALSE;
    }

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "GW: Genesis is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
    if (!fCopyOver)
          control = init_socket( port );
    boot_db( );
    sprintf( log_buf, "GW: Genesis is ready to rock on port %d.", port );
    log_string( log_buf );
    if (fCopyOver)
        copyover_recover();
    game_loop_unix( control );
    close( control );
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    long fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close( fd );
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close( fd );
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror( "Init_socket: bind" );
	close( fd );
	exit( 1 );
    }

    if ( listen( fd, 3 ) < 0 )
    {
	perror( "Init_socket: listen" );
	close( fd );
	exit( 1 );
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor	= 0;
    dcon.character	= NULL;
    dcon.connected	= CON_GET_NAME;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize	= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.next		= descriptor_list;
    dcon.pEdit		= NULL;	/* OLC */
    dcon.pString	= NULL;	/* OLC */
    descriptor_list	= &dcon;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( &dcon, help_greeting+1, 0 );
	else
	    write_to_buffer( &dcon, help_greeting  , 0 );
    }

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

        combat_status_update();

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character != NULL )
			do_autosave( d->character,"" );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		d->character->wait -= 1 ;
		if (d->character->wait < 0) d->character->wait = 0;
		continue;
	    }

            if (d->character != NULL && IS_COMBAT_PAUSED(d->character))
                continue;

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

	        if ( d->pString != NULL ) 
		    string_add( d->character,  d->incomm );	/* OLC */
	        else
		{
	            switch ( d->connected )
	            {
	            case CON_PLAYING:
	                interpret( d->character, d->incomm );
	                break;
	            case CON_AEDITOR:
	                aedit( d->character, d->incomm );
	                break;
	            case CON_REDITOR:
	                redit( d->character, d->incomm );
	                break;
	            case CON_OEDITOR:
	                oedit( d->character, d->incomm );
	                break;
	            case CON_MEDITOR:
	                medit( d->character, d->incomm );
	                break;
	            default:
	                nanny( d, d->incomm );
	                break;
	            }
		}
		d->incomm[0] = '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL )
			do_autosave( d->character, "" );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    long delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
		    if ( dcon.character != NULL )
			do_autosave( d->character,"" );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)

void excessive_cpu(long blx)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next;

	if ( !IS_NPC(vch) )
	{
	    send_to_char("Mud frozen: Autosave and quit.  The mud will reboot in 2 seconds.\n\r",vch);
	    interpret( vch, "quit" );
	}
    }
    exit(1);
}

/*
 * char_list contains world characters, not the partial password/MOTD shells.
 * Save link-dead players and switched originals too, but never login shells.
 * Editors/notes are fully loaded states; detach the descriptor temporarily so
 * save_char_obj's normal login-state guard does not skip these players or
 * redirect a save through a switched descriptor. Restore it before closing.
 * This is normal execution only, never signal-handler or fatal-crash recovery.
 */
static void save_shutdown_players( void )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
        DESCRIPTOR_DATA *d = ch->desc;

        if ( IS_NPC(ch) || ch->pcdata == NULL || ch->in_room == NULL
        || ch->name == NULL || ch->name[0] == '\0' || ch->level < 2 )
            continue;
        if ( d != NULL && d->connected != CON_PLAYING
        && d->connected != CON_AEDITOR && d->connected != CON_REDITOR
        && d->connected != CON_OEDITOR && d->connected != CON_MEDITOR
        && !(d->connected >= CON_NOTE_TO && d->connected <= CON_NOTE_FINISH) )
            continue;

        ch->desc = NULL;
        save_char_obj( ch );
        ch->desc = d;
    }
}

static void shutdown_from_signal( void )
{
    merc_down = TRUE;
    log_string( "Stop signal received: saving loaded players and closing connections." );
    save_shutdown_players();
    while ( descriptor_list != NULL )
    {
        /* Do not generate prompts or wait for queued output during stop. */
        descriptor_list->outtop = 0;
        close_socket( descriptor_list );
    }
}

void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down && !shutdown_requested )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	long maxdesc;

        combat_status_update();

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}


	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    /* select's sets are unspecified after EINTR; rebuild next loop. */
	    if ( errno == EINTR )
		continue;
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}
	if ( shutdown_requested )
	    break;

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    new_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character )
		    do_autosave( d->character,"" );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL )
			do_autosave( d->character,"" );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		d->character->wait -= 1;
                if (d->character->wait < 0) d->character->wait = 0;
		continue;
	    }

            if (d->character != NULL && IS_COMBAT_PAUSED(d->character))
                continue;

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

	        if ( d->pString != NULL ) 
		    string_add( d->character,  d->incomm );	/* OLC */
	        else
		{
	            switch ( d->connected )
	            {
	            case CON_PLAYING:
	                interpret( d->character, d->incomm );
	                break;
	            case CON_AEDITOR:
	                aedit( d->character, d->incomm );
	                break;
	            case CON_REDITOR:
	                redit( d->character, d->incomm );
	                break;
	            case CON_OEDITOR:
	                oedit( d->character, d->incomm );
	                break;
	            case CON_MEDITOR:
	                medit( d->character, d->incomm );
	                break;
	            default:
	                nanny( d, d->incomm );
	                break;
	            }
		}
		d->incomm[0]	= '\0';
	    }
	}


	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL )
			do_autosave( d->character,"" );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((long) last_time.tv_usec) - ((long) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((long) last_time.tv_sec ) - ((long) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		/* EINTR ends this pulse's sleep, not the server. The next
		 * loop check handles a stop, or resumes after other signals.
		 */
		if ( !shutdown_requested
		&& select( 0, NULL, NULL, NULL, &stall_time ) < 0 && errno != EINTR )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    if ( shutdown_requested )
        shutdown_from_signal();

    return;
}
#endif



#if defined(unix)
void new_descriptor( int control )
{
    static DESCRIPTOR_DATA d_zero;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    struct hostent *from;
    long desc;
    long size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, (socklen_t *) &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, (socklen_t *) &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    if ( descriptor_free == NULL )
    {
	dnew		= alloc_perm( sizeof(*dnew) );
    }
    else
    {
	dnew		= descriptor_free;
	descriptor_free	= descriptor_free->next;
    }

    *dnew		= d_zero;
    dnew->descriptor	= desc;
    dnew->character	= NULL;
    dnew->connected	= CON_GET_NAME;
    dnew->outsize	= 2000;
    dnew->pEdit		= NULL;	/* OLC */
    dnew->pString	= NULL;	/* OLC */
    dnew->outbuf	= alloc_mem( dnew->outsize );

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, (socklen_t *)&size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	long addr;

	addr = ntohl( sock.sin_addr.s_addr );	
       sprintf( buf, "%li.%li.%li.%li",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	dnew->host = str_dup( from ? from->h_name : buf );
    }

    sprintf( log_buf, "Sock.sinaddr:  %s", buf );
    log_string( log_buf );
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_suffix( pban->name, dnew->host ) || !str_prefix(pban->name, dnew->host) )
	{
	    write_to_descriptor( desc,
		"Your site has been banned from this Mud.\n\r", 0 );
	    log_string("Site banned.");
	    close( desc );
	    free_string( dnew->host );
	    free_mem( dnew->outbuf, dnew->outsize );
	    dnew->next		= descriptor_free;
	    descriptor_free	= dnew;
	    return;
	}
    }

    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( dnew, help_greeting+1, 0 );
	else
	    write_to_buffer( dnew, help_greeting  , 0 );
    }

    return;
}
#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    if ( dclose->connected < CON_PLAYING )
    {
	dclose->pEdit = NULL;
	dclose->connected = CON_PLAYING;
    }
    if ( dclose->character != NULL && dclose->connected == CON_PLAYING &&
	IS_NPC(dclose->character) ) do_return(dclose->character,"");
/*
    if ( dclose->character != NULL 
	&& dclose->connected == CON_PLAYING
	&& !IS_NPC(dclose->character)
	&& dclose->character->pcdata != NULL
	&& dclose->character->pcdata->obj_vnum != 0
	&& dclose->character->pcdata->chobj != NULL)
	    extract_obj(dclose->character->pcdata->chobj);
*/
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );

        /* If ch is writing note or playing, just lose link otherwise clear char */
        if ( (dclose->connected == CON_PLAYING) ||
	 ((dclose->connected >= CON_NOTE_TO) && (dclose->connected <= CON_NOTE_FINISH)))
	{
	    if (IS_NPC(ch) || ch->pcdata->obj_vnum == 0)
	    	act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    ch->desc = NULL;
	}
	else free_char( dclose->character );
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_string( dclose->host );

    /* RT socket leak fix */
    free_mem( dclose->outbuf, dclose->outsize );

    dclose->next	= descriptor_free;
    descriptor_free	= dclose;
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}

/* For a better kickoff message :) KaVir */
void close_socket2( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    if ( dclose->connected < CON_PLAYING )
    {
	dclose->pEdit = NULL;
	dclose->connected = CON_PLAYING;
    }
    if ( dclose->character != NULL && dclose->connected == CON_PLAYING &&
	IS_NPC(dclose->character) ) do_return(dclose->character,"");
/*
    if ( dclose->character != NULL 
	&& dclose->connected == CON_PLAYING
	&& !IS_NPC(dclose->character)
	&& dclose->character->pcdata != NULL
	&& dclose->character->pcdata->obj_vnum != 0
	&& dclose->character->pcdata->chobj != NULL)
	    extract_obj(dclose->character->pcdata->chobj);
*/
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	if ( dclose->connected == CON_PLAYING )
	{
	    ch->desc = NULL;
	}
	else
	{
	    free_char( dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_string( dclose->host );

    /* RT socket leak fix */
    free_mem( dclose->outbuf, dclose->outsize );

    dclose->next	= descriptor_free;
    descriptor_free	= dclose;
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}


bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    long iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	if (d != NULL && d->character != NULL)
	    sprintf( log_buf, "%s input overflow!", d->character->lasthost );
	else
	    sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );

	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	long c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; iStart < sizeof(d->inbuf) - 10; )
    {
	long nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
    d->inbuf[iStart] = '\0';
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
else if ( errno == EINTR )
    continue;
else if ( errno == EWOULDBLOCK || errno == EAGAIN )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    long i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 40 )
	    {
		if (d != NULL && d->character != NULL)
		    sprintf( log_buf, "%s input spamming!", d->character->lasthost );
		else
		    sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
	    }
	}
    }

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
static long write_descriptor_bytes( long desc, const char *txt, long length );

bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;
    long written;

    /* Overflow is deferred here: callers of write_to_buffer still own d. */
    if ( d->outtop >= d->outsize )
        return FALSE;

    /*
     * Bust a prompt.
     */
    if ( fPrompt && d->fcommand && d->pString != NULL ) /* OLC */
	write_to_buffer( d, "<[OLC]> ", 0 );
    else if ( fPrompt && d->fcommand && !merc_down &&
              d->connected == CON_PLAYING )
    {
	CHAR_DATA *ch;
	CHAR_DATA *victim;

	ch = d->original ? d->original : d->character;
	if ( IS_SET(ch->act, PLR_BLANK) )
	    write_to_buffer( d, "\n\r", 2 );

	if (IS_SET(ch->act, PLR_PROMPT) && IS_EXTRA(ch, EXTRA_PROMPT))
	    bust_a_prompt( d );
	else if ( IS_SET(ch->act, PLR_PROMPT) )
	{
	    char buf[MAX_STRING_LENGTH];
	    char cond[MAX_INPUT_LENGTH];
	    char hit_str[MAX_INPUT_LENGTH];
	    char mana_str[MAX_INPUT_LENGTH];
	    char exp_str[MAX_INPUT_LENGTH];

	    ch = d->character;
	    if (IS_MORE(ch,MORE_HEAD) || IS_EXTRA(ch,EXTRA_OSWITCH))
	    {
		sprintf(exp_str, "%li", ch->exp);
		COL_SCALE(exp_str, ch, ch->exp, 100);
	        sprintf( buf, "<[%s] [?H ?M ?V]> ",exp_str );
	    }
	    else if (ch->position == POS_FIGHTING)
	    {
	        victim = ch->fighting;
		if (victim->hit < 1 || victim->max_hit < 1)
		{
		    strcpy(cond, "Incapacitated");
		    ADD_COLOUR(ch, cond, L_RED);
		}
		else if ((victim->hit*100/victim->max_hit) < 17)
		{
		    strcpy(cond, "Crippled");
		    ADD_COLOUR(ch, cond, L_RED);
		}
		else if ((victim->hit*100/victim->max_hit) < 34)
		{
		    strcpy(cond, "Mauled");
		    ADD_COLOUR(ch, cond, L_BLUE);
		}
		else if ((victim->hit*100/victim->max_hit) < 50)
		{
		    strcpy(cond, "Wounded");
		    ADD_COLOUR(ch, cond, L_BLUE);
		}
		else if ((victim->hit*100/victim->max_hit) < 67)
		{
		    strcpy(cond, "Injured");
		    ADD_COLOUR(ch, cond, L_GREEN);
		}
		else if ((victim->hit*100/victim->max_hit) < 83)
		{
		    strcpy(cond, "Hurt");
		    ADD_COLOUR(ch, cond, L_GREEN);
		}
		else if ((victim->hit*100/victim->max_hit) < 100)
		{
		    strcpy(cond, "Bruised");
		    ADD_COLOUR(ch, cond, YELLOW);
		}
		else
		{
		    strcpy(cond, "Perfect");
		    ADD_COLOUR(ch, cond, L_CYAN);
		}
		sprintf(hit_str, "%li", ch->hit);
		COL_SCALE(hit_str, ch, ch->hit, ch->max_hit);
		sprintf(mana_str, "%li", ch->mana);
		COL_SCALE(mana_str, ch, ch->mana, ch->max_mana);
		sprintf( buf, "<[%s] [%sH %sM]> ", cond, hit_str, mana_str);
	    }
	    else
	    {
		sprintf(hit_str, "%li", ch->hit);
		COL_SCALE(hit_str, ch, ch->hit, ch->max_hit);
		sprintf(mana_str, "%li", ch->mana);
		COL_SCALE(mana_str, ch, ch->mana, ch->max_mana);
		sprintf(exp_str, "%li", ch->exp);
		COL_SCALE(exp_str, ch, ch->exp, 100);
	        sprintf( buf, "<[%sX] [%sH %sM]> ",exp_str, hit_str, mana_str);
	    }
	    write_to_buffer( d, buf, 0 );
	}

	if ( IS_SET(ch->act, PLR_TELNET_GA) )
	    write_to_buffer( d, go_ahead_str, 0 );
    }
    /* A retry of this output is not another completed command. */
    if ( fPrompt )
        d->fcommand = FALSE;

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop >= d->outsize )
return FALSE;
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->snoop_by->character != NULL &&  IS_IMM(d->snoop_by->character))
	{
	    write_to_buffer( d->snoop_by, "% ", 2 );
	    write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
	}
    }

    /*
     * OS-dependent output.
     */
    written = write_descriptor_bytes( d->descriptor, d->outbuf, d->outtop );
    if ( written < 0 )
    {
	d->outtop = 0;
	return FALSE;
    }
    d->outtop -= written;
    memmove( d->outbuf, d->outbuf + written, d->outtop );
    d->outbuf[d->outtop] = '\0';
    return TRUE;
}


/*
 * Append onto an output buffer.
 * Color code created by Xkilla, and modified by Duo.
 * Further cleaning by Akuma.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, long length )
{
    char tb[30000], ccode, code1, code2;
    long i, j;
    bool ansi;

    if ( d->outtop >= d->outsize )
        return;
    /*
     * Find length in case caller didn't.
     */
    if (length <= 0) length = strlen(txt);

    if (length >= MAX_STRING_LENGTH )
    {
	bug("Write_to_buffer: Way too big. Closing.", 0);
	return;
    }

    if (d->character != NULL && !IS_NPC(d->character) && IS_SET(d->character->act, PLR_ANSI)) ansi = TRUE;
    else ansi = FALSE;

    /*
     * Initial \n\r if needed.
     */
    if (d->outtop == 0 && !d->fcommand)
    {
	d->outbuf[0] = '\n';
	d->outbuf[1] = '\r';
	d->outtop = 2;
    }

    /*
     * Expand d->outbuf for ansi info.
     */
    j = 0;
    code1 = '0';
    code2 = '7';
    ccode = '3';
    tb[j] = '\0';
    for (i = 0; i < length; i++)
    {
	if (txt[i] == '#')
	{
    /* An explicit length need not include a terminating NUL. */
    if ( i + 1 >= length )
    {
        tb[j++] = '#';
        continue;
    }
	    i++;
	    if (txt[i] == '#') {tb[j++] = '#'; continue;}
	    if (txt[i] == '-') {tb[j++] = '~'; continue;}
	    if (txt[i] == '+') {tb[j++] = '%'; continue;}

	    if (!ansi) continue;

	    /*
	     * Text modifiers.  Bold, Italic, etc...
	     */
//	    if (txt[i] == ' ') {tb[j++] = 27; tb[j++] = '['; tb[j++] = '0'; tb[j++] = 'm'; continue;} /* Kill? */
	    if (txt[i] == 't') {tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = 'm'; continue;} /* Bright */
	    if (txt[i] == 'd') {tb[j++] = 27; tb[j++] = '['; tb[j++] = '2'; tb[j++] = 'm'; continue;} /* Dark */
	    if (txt[i] == 'i') {tb[j++] = 27; tb[j++] = '['; tb[j++] = '3'; tb[j++] = 'm'; continue;} /* Italics */
	    if (txt[i] == 'u') {tb[j++] = 27; tb[j++] = '['; tb[j++] = '4'; tb[j++] = 'm'; continue;} /* Underlined */
//	    if (txt[i] == ' ') {tb[j++] = 27; tb[j++] = '['; tb[j++] = '5'; tb[j++] = 'm'; continue;} /* Blinking */
//	    if (txt[i] == ' ') {tb[j++] = 27; tb[j++] = '['; tb[j++] = '6'; tb[j++] = 'm'; continue;} /* Italics? */
	    if (txt[i] == 'h') {tb[j++] = 27; tb[j++] = '['; tb[j++] = '7'; tb[j++] = 'm'; continue;} /* Highlight */

	    /*
	     * First half of normal.  Kills text to default.
	     */
	
	    if (txt[i] == 'n') {tb[j++] = 27; tb[j++] = '['; tb[j++] = '0'; tb[j++] = 'm';}
	    switch (txt[i])
	    {
		default: continue;
		case '0': case 'B':	code1 = '1'; code2 = '0'; break;	/* Start of bright colors */
		case '1': case 'R':	code1 = '1'; code2 = '1'; break;
		case '2': case 'G':	code1 = '1'; code2 = '2'; break;
		case '3': case 'y':	code1 = '1'; code2 = '3'; break;
		case '4': case 'L':	code1 = '1'; code2 = '4'; break;
		case '5': case 'P':	code1 = '1'; code2 = '5'; break;
		case '6': case 'C':	code1 = '1'; code2 = '6'; break;
		case '7': case 'W':	code1 = '1'; code2 = '7'; break;
		case 'b':		code1 = '0'; code2 = '0'; break;	/* Start of regular colors */
		case 'r':		code1 = '0'; code2 = '1'; break;
		case 'g':		code1 = '0'; code2 = '2'; break;
		case 'o':		code1 = '0'; code2 = '3'; break;
		case 'l':		code1 = '0'; code2 = '4'; break;
		case 'p':		code1 = '0'; code2 = '5'; break;
		case 'c':		code1 = '0'; code2 = '6'; break;
		case 'n':		code1 = '0'; code2 = '7'; break;	/* Normal part 2 */
		case 's':		code1 = '0' + number_range(0, 1);	/* Random color */
					code2 = '0' + number_range(0, 7); break;
	    }

	    tb[j++] = 27;
	    tb[j++] = '[';
	    tb[j++] = code1;
	    tb[j++] = ';';
	    tb[j++] = ccode;
	    tb[j++] = code2;
	    tb[j++] = 'm';
	}
	else tb[j++] = txt[i];
    }

    {
	tb[j++] = 27;
	tb[j++] = '[';
	tb[j++] = '0';
	tb[j++] = ';';
	tb[j++] = ccode;
	tb[j++] = '7';
	tb[j++] = 'm';
    }
    tb[j]   = '\0';

    length = j;

    /*
     * Expand the buffer as needed.
     */
    while (d->outtop + length >= d->outsize)
    {
	char *outbuf;

	if (d->outsize >= 128000)
	{
	    bug("Buffer overflow. Closing.\n\r", 0);
    /* Let process_output close it, without invalidating its caller. */
    d->outtop = d->outsize;
	    return;
	}
	outbuf = alloc_mem(2 * d->outsize);
	strncpy(outbuf, d->outbuf, d->outtop);
	free_mem(d->outbuf, d->outsize);
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }
    /*
     * Copy.
     */
    strncpy(d->outbuf + d->outtop, tb, length);
    d->outtop += length;
    d->outbuf[d->outtop] = '\0';
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
static long write_descriptor_bytes( long desc, const char *txt, long length )
{
    long iStart;
    long nWrite;
    long nBlock;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    for ( iStart = 0; iStart < length; )
    {
	nBlock = UMIN( length - iStart, 4096 );
 nWrite = write( desc, txt + iStart, nBlock );
if ( nWrite > 0 )
    iStart += nWrite;
else if ( nWrite < 0 && errno == EINTR )
    continue;
else if ( nWrite < 0 && (errno == EWOULDBLOCK || errno == EAGAIN) )
    break;
else
    return -1;
    } 

    return iStart;
}

bool write_to_descriptor( long desc, char *txt, long length )
{
    if ( length <= 0 )
        length = strlen(txt);
    return write_descriptor_bytes( desc, txt, length ) == length;
}

/*
 * Old player files may contain plaintext. Never treat an encoded password
 * itself as a plaintext credential, even if its migration flag is missing.
 */
bool password_matches( const char *argument, const char *stored,
                       bool allow_plaintext )
{
    char *encoded;
    bool looks_encoded;

    if ( argument == NULL || stored == NULL || stored[0] == '\0' )
        return FALSE;
    encoded = crypt( argument, stored );
    if ( encoded != NULL && encoded[0] != '*' &&
         !strcmp( encoded, stored ) )
        return TRUE;

    looks_encoded = stored[0] == '$' || stored[0] == '_' ||
        stored[0] == '*' || stored[0] == '!' ||
        (strlen(stored) == 13 &&
         strspn(stored, "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == 13);
    return allow_plaintext && !looks_encoded && !strcmp(argument, stored);
}

/*
 * libxcrypt obtains salt entropy from the OS when rbytes is NULL. Cost 5 is
 * yescrypt's supported default (about 16 MiB per hash). Failure is fatal to
 * this operation: never retry with DES, a fixed salt, or plaintext.
 * crypt() returns static storage, so copy it before another crypt() call.
 */
static bool password_set( CHAR_DATA *ch, const char *argument )
{
    char setting[CRYPT_GENSALT_OUTPUT_SIZE];
    char *encoded;
    char *replacement;

    if ( argument == NULL || argument[0] == '\0' ||
         crypt_gensalt_rn("$y$", 5, NULL, 0, setting, sizeof(setting)) == NULL ||
         strncmp(setting, "$y$", 3) )
        return FALSE;

    encoded = crypt(argument, setting);
    if ( encoded == NULL || strncmp(encoded, "$y$", 3) ||
         strspn(encoded, "$./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")
             != strlen(encoded) ||
         strlen(encoded) <= strlen(setting) ||
         strncmp(encoded, setting, strlen(setting)) ||
         encoded[strlen(setting)] != '$' )
        return FALSE;

    replacement = str_dup(encoded);
    if ( replacement == NULL )
        return FALSE;
    free_string(ch->pcdata->pwd);
    ch->pcdata->pwd = replacement;
    SET_BIT(ch->extra, EXTRA_NEWPASS);
    return TRUE;
}

/*
 * Only call on the actual player, selected before reconnect/takeover or
 * after a full load; switched descriptors authenticate their original.
 * Saving a load_char_short() shell would discard inventory. Keep the pending
 * save on the descriptor until CON_PLAYING, as save_char_obj requires.
 */
static bool password_upgrade( DESCRIPTOR_DATA *d, const char *argument )
{
    CHAR_DATA *ch = d->original ? d->original : d->character;

    if ( ch == NULL || ch->pcdata == NULL ||
         !password_matches(argument, ch->pcdata->pwd,
                           !IS_EXTRA(ch, EXTRA_NEWPASS)) )
        return FALSE;

    if ( strncmp(ch->pcdata->pwd, "$y$", 3) )
    {
        if ( !password_set(ch, argument) )
            return FALSE;
        d->password_save_pending = TRUE;
    }
    else if ( !IS_EXTRA(ch, EXTRA_NEWPASS) )
    {
        SET_BIT(ch->extra, EXTRA_NEWPASS);
        d->password_save_pending = TRUE;
    }
    return TRUE;
}

static void password_save_if_ready( DESCRIPTOR_DATA *d )
{
    if ( d->password_save_pending && d->connected == CON_PLAYING )
    {
        CHAR_DATA *ch = d->original ? d->original : d->character;
        if ( save_char_obj_checked(ch) )
            d->password_save_pending = FALSE;
    }
}

/* Validate and prepare the live player's hash before changing any ownership.
 * The short-login shell can be stale (notably after a failed disk save).
 * Never let that stale password disconnect an authenticated live session. */
static bool password_prepare_live( DESCRIPTOR_DATA *d, const char *argument )
{
    CHAR_DATA *ch;
    DESCRIPTOR_DATA *old;
    DESCRIPTOR_DATA candidate = {0};

    for ( ch = char_list; ch != NULL; ch = ch->next )
        if ( !IS_NPC(ch) && !IS_EXTRA(ch, EXTRA_SWITCH) &&
             ch->desc == NULL && !str_cmp(d->character->name, ch->name) )
            break;
    if ( ch == NULL )
        for ( old = descriptor_list; old != NULL; old = old->next )
            if ( old != d && old->character != NULL &&
                 old->connected != CON_GET_NAME &&
                 old->connected != CON_GET_OLD_PASSWORD )
            {
                CHAR_DATA *player = old->original ? old->original : old->character;
                if ( !str_cmp(d->character->name, player->name) )
                {
                    ch = player;
                    break;
                }
            }
    if ( ch == NULL )
        return TRUE;
    candidate.character = ch;
    if ( !password_upgrade(&candidate, argument) )
        return FALSE;
    d->password_save_pending |= candidate.password_save_pending;
    return TRUE;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char kav[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    char *strtime;
    char *lasttime;
    bool fOld; 

    /* Delete leading spaces UNLESS character is writing a note */
    if (d->connected != CON_NOTE_TEXT)
    {
	while ( isspace(*argument) )
	    argument++;
    }

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %li.", d->connected );
	close_socket( d );
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}

	sprintf(kav,"%s trying to connect.", argument);
	log_string( kav );
	fOld = load_char_short( d, argument );
	ch   = d->character;
if (!fOld && clan_identity_reserved(argument))
{
    write_to_buffer(d, "That name is reserved by clan records; contact staff.\n\r", 0);
    close_socket(d);
    return;
}

	if ( fOld && ch->lasthost != NULL && strlen(ch->lasthost) > 1 &&
		ch->lasttime != NULL && strlen(ch->lasttime) > 1 )
	{
//	    sprintf(kav,"Last connected from %s at %s\n\r",ch->lasthost,ch->lasttime);
	    sprintf(kav,"Last connected at %s\n\r",ch->lasttime);
	    write_to_buffer( d, kav, 0 );
	}
	else if ( fOld && ch->lasthost != NULL && strlen(ch->lasthost) > 1 )
	{
	    sprintf(kav,"Last connected from %s.\n\r",ch->lasthost);
	    write_to_buffer( d, kav, 0 );
	}

	if ( !IS_NPC(ch) && ch->pcdata->denydate > 0 )
	{
	    if (!play_yet(ch))
	    {
		sprintf( log_buf, "Denying access to %s@%s.", argument, ch->lasthost );
		log_string( log_buf );
		close_socket( d );
		return;
	    }
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    /* Check max number of players - KaVir */

	    DESCRIPTOR_DATA *dcheck;
	    DESCRIPTOR_DATA *dcheck_next;
	    long countdesc = 0;
	    long max_players = 150;

	    for (dcheck = descriptor_list; dcheck != NULL; dcheck = dcheck_next)
	    {
		dcheck_next = dcheck->next;
		if (dcheck->character != NULL && dcheck != d)
		{
		    if (!IS_NPC(dcheck->character) && ch->level < LEV_SLE 
			&& strlen(dcheck->character->name) > 0 
			&& !str_cmp(dcheck->character->name, ch->name))
		    {
			write_to_buffer( d, "Sorry, there is already someone of your name connected.\n\r", 0 );
			close_socket( d );
			return;
		    }
		}
		countdesc++;
	    }

	    if ( countdesc > max_players && !IS_BUI(ch) )
	    {
		write_to_buffer( d, "Too many players connected, please try again in a couple of minutes.\n\r", 0 );
		close_socket( d );
		return;
	    }

	    if ( wizlock && !IS_BUI(ch) )
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	    if (countdesc > high_number_login) high_number_login = countdesc;
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Please enter password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
            if ( newbielock )
            {
                write_to_buffer( d, "\n\r\n\rSorry,  No new characters allowed right now.\n\r\n\r\n\r", 0 );
                close_socket( d );
                return;
            }
	    /* New player */
	    sprintf( buf, "You want %s engraved on your tombstone (Y/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif


if ( ch == NULL || ch->pcdata == NULL ||
     !password_matches( argument, ch->pcdata->pwd,
                        !IS_EXTRA(ch,EXTRA_NEWPASS) ) )
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );

if ( !password_prepare_live(d, argument) )
{
    write_to_buffer(d, "Password authentication or upgrade failed.\n\r", 0);
    close_socket(d);
    return;
}

	if ( check_reconnect( d, ch->name, TRUE ) ||
	     check_playing( d, ch->name ) )
        {
            /* The live player was authenticated/upgraded before takeover. */
            password_save_if_ready(d);
            return;
        }
/*
	if ( check_kickoff( d, ch->name, TRUE ) )
	    return;

	** Avoid nasty duplication bug - KaVir */
	if (IS_AWA(ch))
	{
	    strcpy(kav,ch->name);
	    free_char(d->character);
	    d->character = NULL;
	    fOld = load_char_obj( d, kav );
	    ch   = d->character;
	}

        if ( !password_upgrade(d, argument) )
        {
            write_to_buffer(d, "Password upgrade failed. Please try again later.\n\r", 0);
            close_socket(d);
            return;
        }

	if (ch->lasthost != NULL) free_string(ch->lasthost);
	if (ch->desc != NULL && ch->desc->host != NULL)
	{
	    if                                                                 (!str_cmp(ch->name,"Pandora")) ch->lasthost = str_dup("mhd1.moorhead.msus.edu");
	    else if                                                            (!str_cmp(ch->name,"Zadmar")) ch->lasthost = str_dup("hell.demon.co.uk");
	    else if                                                            (!str_cmp(ch->name,"Trina")) ch->lasthost = str_dup("scitsc.wlv.ac.uk");
	    else ch->lasthost = str_dup(ch->desc->host);
	}
	else
	    ch->lasthost = str_dup("(unknown)");

	if ( strlen(ch->lasttime) >= 11 )
	{
	    lasttime = str_dup( ch->lasttime );
	    lasttime[11] = '\0';
	    strtime = ctime( &current_time );
	    strtime[11] = '\0';
	}

	strtime = ctime( &current_time );
	strtime[strlen(strtime)-1] = '\0';
	free_string(ch->lasttime);
	ch->lasttime = str_dup( strtime );
	sprintf( log_buf, "%s@%s has connected.", ch->name, ch->lasthost );
	log_string( log_buf );

	if (ch->level > LEV_ST) ch->level = LEV_ST;

        if (IS_IMM(ch))
        {
            write_to_buffer( d, "Do you wish to enter Wizinvis, Incognito, or Normal ( W / I / N ): ", 0 );
            d->connected = CON_GET_INVIS;
        }
	else
	{
            do_help(ch,"motd");
	    write_to_buffer(d,"\n\rPress return to continue.\n\r",0);
            d->connected = CON_READ_MOTD;
        }
	break;


    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "Give me a password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

        if ( !password_set(ch, argument) )
        {
            write_to_buffer(d, "Password encryption failed. Try again.\n\rPassword: ", 0);
            return;
        }

	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

if ( !password_matches( argument, ch->pcdata->pwd, FALSE ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );
	write_to_buffer( d, "What is your sex (M/F)? ", 0 );
	d->connected = CON_GET_NEW_SEX;
	break;

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}
	    write_to_buffer( d, echo_on_str, 0 );
        write_to_buffer( d, "Does your terminal support ANSI (y/n)? ", 0);
        d->connected = CON_GET_NEW_ANSI;
        break;

    case CON_GET_NEW_ANSI:
        switch ( argument[0] )
        {
        case 'y': case 'Y': SET_BIT(ch->act,PLR_ANSI);    break;
        case 'n': case 'N': break;
        default:
            write_to_buffer( d, "Does your terminal support ANSI? (y/n)?", 0 );
            return;
        }
	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	write_to_buffer( d, "\n\r", 2 );
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
	break;

    case CON_GET_INVIS:
        switch ( argument[0] )
        {
        case 'w': case 'W':
                                SET_BIT(ch->act, PLR_WIZINVIS);
                                REMOVE_BIT(ch->act, PLR_WIZINCOG);
                                d->connected = CON_READ_MOTD;
                            break;
        case 'i': case 'I':
                                SET_BIT(ch->act, PLR_WIZINCOG);
                                REMOVE_BIT(ch->act, PLR_WIZINVIS);
                                d->connected = CON_READ_MOTD;
                            break;
        case 'n': case 'N':
                                REMOVE_BIT(ch->act, PLR_WIZINVIS);
                                REMOVE_BIT(ch->act, PLR_WIZINCOG);
                                d->connected = CON_READ_MOTD;
                            break;
        default:
                send_to_char("\n\r", ch );
                send_to_char("(W)izinvis     or     (I)ncognito     or     (N)ormal\n\r", ch );
            return;
        }


    case CON_READ_MOTD:
	write_to_buffer(d, "\n\rWelcome to Godwars: Genesis.\n\r", 0);
	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;
	total_number_login++;

	if (IS_WEREWOLF(ch)) ch->vampgen = (7 - ch->pcdata->rank);
	if (IS_HIGHLANDER(ch) && ch->vampgen == 0) ch->vampgen = 2;

	if (!CAN_PK(ch) && get_hours(ch) >= AGE_PK)
	{
	    SET_BIT(ch->pcdata->flag, FLAG_PK);
	    stc("You are no longer a newbie, and can now be attacked by other players.\n\r", ch);
	}

	if (ch->level < LEV_SLE)
	{
	    ch->level		= LEV_SLE;
	    ch->hit		= ch->max_hit;
	    ch->mana		= ch->max_mana;
	    set_title(ch, "the sleeper");
	    ch->exp		= 0;
	    ch->class		= CLASS_NONE;
            ch->pcdata->board	= &boards[DEFAULT_BOARD];
	    stc("--------------------------------------------------------------------------------\n\r", ch);
	    stc("If you need help, try talking to the spirit of mud school!\n\r", ch);
	    stc("--------------------------------------------------------------------------------\n\r", ch);
	    char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
	    do_look(ch, "auto");
	    /* Newbie EQ */
	    stc("\n\r", ch);
	}
	else if (ch->in_room != NULL)
	{
	    char_to_room(ch, ch->in_room);
	    do_look(ch, "auto");
	}
	else
	{
	    char_to_room(ch, get_room_index( ROOM_VNUM_TEMPLE));
	    do_look(ch, "auto");
	}

	do_board(ch, ""); /* Show board status */

	update_style(ch);
password_save_if_ready(d);
	if (!IS_IMM(ch))
	{
	    sprintf(buf, "#r%s#n #ghas entered the #7G #0E #7N #0E #7S #0I #7S#n.", ch->name);
	    do_info(ch, buf);
	}

	if (!IS_NPC(ch) && ch->pcdata->obj_vnum != 0)
	{
	    bind_char(ch);
	    break;
	}

	act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
	room_text(ch,">ENTER<");

	break;
        /* states for new note system, (c)1995-96 erwin@pip.dknet.dk */
        /* ch MUST be PC here; have nwrite check for PC status! */

        case CON_NOTE_TO:
                handle_con_note_to (d, argument);
                break;

        case CON_NOTE_SUBJECT:
                handle_con_note_subject (d, argument);
                break; /* subject */

        case CON_NOTE_EXPIRE:
                handle_con_note_expire (d, argument);
                break;

        case CON_NOTE_TEXT:
                handle_con_note_text (d, argument);
                break;

        case CON_NOTE_FINISH:
                handle_con_note_finish (d, argument);
                break;
    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words.
     */
    if ( is_name( name, "all auto immortal self someone gaia imms imm imp immo imps !" ) )
	return FALSE;

    if (is_in(name,"|shit**voadkin*ayreon*fuck*cunt*assh*bitch*suck*dick*damn*damnit*yousuck*motherfucker*bastard*!*self*myself|me|"))
	return FALSE;

    /*
     * Length restrictions.
     */
    if ( strlen(name) <  3 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	long iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }
     */

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch) && !IS_EXTRA(ch, EXTRA_SWITCH)
	&& ( !fConn || ch->desc == NULL )
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
if ( IS_EXTRA(ch, EXTRA_NEWPASS) )
    SET_BIT(d->character->extra, EXTRA_NEWPASS);
else
    REMOVE_BIT(d->character->extra, EXTRA_NEWPASS);
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char( "Reconnecting.\n\r", ch );
	    	if (IS_NPC(ch) || ch->pcdata->obj_vnum == 0)
		    act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s@%s reconnected.", ch->name, ch->lasthost );
		log_string( log_buf );
		d->connected = CON_PLAYING;
                /* Inform the character of a note in progress and the possbility of continuation! */
                if (ch->pcdata->in_progress)
                        send_to_char ("You have a note in progress. Type NWRITE to continue it.\n\r",ch);
	    }
	    return TRUE;
	}
    }

    return FALSE;
}

/*
 * Kick off old connection.  KaVir.
 */
bool check_kickoff( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&& ( !fConn || ch->desc == NULL )
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char( "You take over your body, which was already in use.\n\r", ch );
		act( "...$n's body has been taken over by another spirit!", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s@%s kicking off old link.", ch->name, ch->lasthost );
		log_string( log_buf );
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing - KaVir.
 * Using kickoff code from Malice, as mine is v. dodgy.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold != NULL; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    char	buf [MAX_STRING_LENGTH];
	    if ( d->character != NULL )
	    {
		free_char( d->character );
		d->character = NULL;
	    }
	    send_to_char("This body has been taken over!\n\r",dold->character);
	    d->connected = dold->connected;
    d->password_save_pending = d->password_save_pending || dold->password_save_pending;
	    d->character = dold->character;
    d->original = dold->original;
	    d->character->desc = d;
    if ( d->original != NULL )
        d->original->desc = NULL; /* Preserve do_switch/do_return ownership. */
	    send_to_char( "You take over your body, which was already in use.\n\r", d->character );
	    act( "$n doubles over in agony and $s eyes roll up into $s head.", d->character, NULL, NULL, TO_ROOM );
	    act( "...$n's body has been taken over by another spirit!", d->character, NULL, NULL, TO_ROOM );
	    dold->character=NULL;
    dold->original=NULL;
	    
	    sprintf(buf,"Kicking off old connection %s@%s",d->character->name,d->host);
	    log_string(buf);
	    close_socket(dold);	/*Slam the old connection into the ether*/
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    CHAR_DATA *wizard;
    CHAR_DATA *familiar;

    if ( ch->desc == NULL && IS_NPC(ch) && (wizard = ch->wizard) != NULL )
    {
	if (!IS_NPC(wizard) && (familiar = wizard->pcdata->familiar) != NULL 
	    && familiar == ch && ch->in_room != wizard->in_room)
	{
	    SET_BIT(wizard->extra, EXTRA_CAN_SEE);
	    send_to_char("[ ",wizard);
	    if ( txt != NULL && wizard->desc != NULL )
		write_to_buffer( wizard->desc, txt, strlen(txt) );
	    REMOVE_BIT(wizard->extra, EXTRA_CAN_SEE);
	}
    }

/* The following is the original contents of void send_to_char */
    if ( txt != NULL && ch->desc != NULL )
	write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/*
 * Write to all characters.
 */
void send_to_all_char( const char *text )
{
    DESCRIPTOR_DATA *d;

    if ( text == NULL)
        return;
    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING )
	    send_to_char( text, d->character );

    return;
}


/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( DESCRIPTOR_DATA *d )
{
         CHAR_DATA *ch;
         CHAR_DATA *victim;
         CHAR_DATA *tank;
   const char      *str;
   const char      *i;
         char      *point;
         char       buf  [ MAX_STRING_LENGTH ];
         char       buf2 [ MAX_STRING_LENGTH ];
	 bool       is_fighting = TRUE;

   if ( ( ch = d->character ) == NULL ) return;
   if ( ch->pcdata == NULL )
   {
      send_to_char( "\n\r\n\r", ch );
      return;
   }
   if ( ch->position == POS_FIGHTING && ch->cprompt[0] == '\0' )
   {
      if ( ch->prompt[0] == '\0' )
      {
         send_to_char( "\n\r\n\r", ch );
         return;
      }
      is_fighting = FALSE;
   }
   else if ( ch->position != POS_FIGHTING && ch->prompt[0] == '\0' )
   {
      send_to_char( "\n\r\n\r", ch );
      return;
   }

   point = buf;
   if ( ch->position == POS_FIGHTING && is_fighting )
      str = d->original ? d->original->cprompt : d->character->cprompt;
   else
      str = d->original ? d->original->prompt : d->character->prompt;
   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default :
            i = " "; break;
         case 'h' :
            sprintf( buf2, "%li", ch->hit );
            COL_SCALE(buf2, ch, ch->hit, ch->max_hit);
            i = buf2; break;
         case 'H' :
            sprintf( buf2, "%li", ch->max_hit );
            ADD_COLOUR(ch, buf2, L_CYAN);
            i = buf2; break;
         case 'm' :
            sprintf( buf2, "%li", ch->mana                              );
            COL_SCALE(buf2, ch, ch->mana, ch->max_mana);
            i = buf2; break;
         case 'M' :
            sprintf( buf2, "%li", ch->max_mana                          );
            ADD_COLOUR(ch, buf2, L_CYAN);
            i = buf2; break;
         case 'x' :
            sprintf( buf2, "%li", ch->exp                               );
            COL_SCALE(buf2, ch, ch->exp, 100);
            i = buf2; break;
         case 'g' :
            sprintf( buf2, "%li", ch->gold                              );
            ADD_COLOUR(ch, buf2, L_CYAN);
            i = buf2; break;
         case 'q' :
            sprintf( buf2, "%li", ch->pcdata->quest                     );
            ADD_COLOUR(ch, buf2, L_CYAN);
            i = buf2; break;
         case 'f' :
            if ( ( victim = ch->fighting ) == NULL )
            {
	       strcpy( buf2, "N/A" );
               ADD_COLOUR(ch, buf2, L_CYAN);
            }
            else
            {
               if (victim->hit < 1 || victim->max_hit < 1)
               {strcpy(buf2, "Incapacitated");ADD_COLOUR(ch, buf2, L_RED);}
               else if ((victim->hit*100/victim->max_hit) < 17)
               {strcpy(buf2, "Crippled");ADD_COLOUR(ch, buf2, L_RED);}
               else if ((victim->hit*100/victim->max_hit) < 34)
               {strcpy(buf2, "Mauled");ADD_COLOUR(ch, buf2, L_BLUE);}
               else if ((victim->hit*100/victim->max_hit) < 50)
               {strcpy(buf2, "Wounded");ADD_COLOUR(ch, buf2, L_BLUE);}
               else if ((victim->hit*100/victim->max_hit) < 75)
               {strcpy(buf2, "Injured");ADD_COLOUR(ch, buf2, L_GREEN);}
               else if ((victim->hit*100/victim->max_hit) < 100)
               {strcpy(buf2, "Hurt");ADD_COLOUR(ch, buf2, L_GREEN);}
               else if ((victim->hit*100/victim->max_hit) >= 100)
               {strcpy(buf2, "Bruised");ADD_COLOUR(ch, buf2, YELLOW);}
               else if ((victim->hit*100/victim->max_hit) >= 100)
               {strcpy(buf2, "Perfect");ADD_COLOUR(ch, buf2, L_CYAN);}
            }
            i = buf2; break;
         case 'F' :
            if ( ( victim = ch->fighting ) == NULL )
            {
	       strcpy( buf2, "N/A" );
               ADD_COLOUR(ch, buf2, L_CYAN);
            }
            else if ( ( tank = victim->fighting ) == NULL )
            {
	       strcpy( buf2, "N/A" );
               ADD_COLOUR(ch, buf2, L_CYAN);
            }
            else
            {
               if (tank->hit < 1 || tank->max_hit < 1)
               {strcpy(buf2, "Incapacitated");ADD_COLOUR(ch, buf2, L_RED);}
               else if ((tank->hit*100/tank->max_hit) < 17)
               {strcpy(buf2, "Crippled");ADD_COLOUR(ch, buf2, L_RED);}
               else if ((tank->hit*100/tank->max_hit) < 34)
               {strcpy(buf2, "Mauled");ADD_COLOUR(ch, buf2, L_BLUE);}
               else if ((tank->hit*100/tank->max_hit) < 50)
               {strcpy(buf2, "Wounded");ADD_COLOUR(ch, buf2, L_BLUE);}
               else if ((tank->hit*100/tank->max_hit) < 75)
               {strcpy(buf2, "Injured");ADD_COLOUR(ch, buf2, L_GREEN);}
               else if ((tank->hit*100/tank->max_hit) < 100)
               {strcpy(buf2, "Hurt");ADD_COLOUR(ch, buf2, L_GREEN);}
               else if ((tank->hit*100/tank->max_hit) >= 100)
               {strcpy(buf2, "Bruised");ADD_COLOUR(ch, buf2, YELLOW);}
               else if ((tank->hit*100/tank->max_hit) >= 100)
               {strcpy(buf2, "Perfect");ADD_COLOUR(ch, buf2, L_CYAN);}
            }
            i = buf2; break;
         case 'n' :
            if ( ( victim = ch->fighting ) == NULL )
	       strcpy( buf2, "N/A" );
            else
            {
               if ( IS_AFFECTED(victim, AFF_POLYMORPH) )
                  strcpy(buf2, victim->morph);
               else if ( IS_NPC(victim) )
                  strcpy(buf2, victim->short_descr);
               else
                  strcpy(buf2, victim->name);
               buf2[0] = UPPER(buf2[0]);
            }
            i = buf2; break;
         case 'N' :
            if ( ( victim = ch->fighting ) == NULL )
	       strcpy( buf2, "N/A" );
            else if ( ( tank = victim->fighting ) == NULL )
	       strcpy( buf2, "N/A" );
            else
            {
               if ( ch == tank )
                  strcpy(buf2, "You");
               else if ( IS_AFFECTED(tank, AFF_POLYMORPH) )
                  strcpy(buf2, tank->morph);
               else if ( IS_NPC(victim) )
                  strcpy(buf2, tank->short_descr);
               else
                  strcpy(buf2, tank->name);
               buf2[0] = UPPER(buf2[0]);
            }
            i = buf2; break;
         case 'e' :
            if ( ( victim = ch->embrace ) == NULL )
	       strcpy( buf2, "N/A" );
            else
            {
               if ( IS_AFFECTED(victim, AFF_POLYMORPH) )
                  strcpy(buf2, victim->morph);
               else if ( IS_NPC(victim) )
                  strcpy(buf2, victim->short_descr);
               else
                  strcpy(buf2, victim->name);
               buf2[0] = UPPER(buf2[0]);
            }
            i = buf2; break;
         case 'E' :
            if ( ( victim = ch->embrace ) == NULL )
            {
	        strcpy( buf2, "N/A" );
                ADD_COLOUR(ch, buf2, L_CYAN);
            }
            else
            {
		sprintf(buf2, "%li", victim->blood[BLOOD_CURRENT]);
		ADD_COLOUR(ch, buf2, L_RED);
            }
            i = buf2; break;
         case 'a' :
            sprintf( buf2, "%s", "" );
            ADD_COLOUR(ch, buf2, L_CYAN);
            i = buf2; break;
         case 'r' :
            if( ch->in_room )
               sprintf( buf2, "%s", ch->in_room->name                  );
            else
               sprintf( buf2, " "                                      );
            ADD_COLOUR(ch, buf2, L_CYAN);
            i = buf2; break;
         case 'R' :
            if (!IS_NPC(ch) && (IS_VAMPIRE(ch) || IS_WEREWOLF(ch)))
            {
               sprintf( buf2, "%li", ch->pcdata->wolf );
               ADD_COLOUR(ch, buf2, D_RED);
            }
            else strcpy( buf2, "0" );
            i = buf2; break;
         case 'b' :
            sprintf( buf2, "%li", ch->beast );
            ADD_COLOUR(ch, buf2, L_CYAN);
            i = buf2; break;
         case 'B' :
            sprintf( buf2, "%li", ch->blood[BLOOD_CURRENT] );
            ADD_COLOUR(ch, buf2, D_RED);
            i = buf2; break;
         case 'c' :
            sprintf( buf2, "%li", char_ac(ch) );
            i = buf2; break;
         case 'C' :
            sprintf( buf2, "%li", ch->blood[BLOOD_POOL] );
            ADD_COLOUR(ch, buf2, D_RED);
            i = buf2; break;
         case 'd' :
            strcpy( buf2, "0" );
            i = buf2; break;
         case 'p' :
            sprintf( buf2, "%li", char_hitroll(ch) );
            COL_SCALE(buf2, ch, char_hitroll(ch), 200);
            i = buf2; break;
         case 'P' :
            sprintf( buf2, "%li", char_damroll(ch) );
            COL_SCALE(buf2, ch, char_damroll(ch), 200);
            i = buf2; break;
         case 'z' :		/* OLC */
            if ( IS_BUI( ch ) )
            {
                switch (ch->desc->connected)
                {
                case CON_AEDITOR:
                    sprintf( buf2, "AEdit" );
                    break;
                case CON_REDITOR:
                    sprintf( buf2, "REdit" );
                    break;
                case CON_OEDITOR:
                    sprintf( buf2, "OEdit" );
                    break;
                case CON_MEDITOR:
                    sprintf( buf2, "MEdit" );
                    break;
                default:
                    buf2[0] = '\0';
                    break;
                }
            }
            else
              sprintf( buf2, " " );
            i = buf2; break;
        case '%' :
            sprintf( buf2, "%%"                                        );
            i = buf2; break;
      } 
      ++str;
      while( ( *point = *i ) != '\0' )
         ++point, ++i;      
   }
   write_to_buffer( d, buf, point - buf );
   return;
}

void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
if ( pArg - arg1 >= sizeof(arg1) - 1 )
{
    send_to_char( "Password argument too long.\n\r", ch );
    return;
}
*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
if ( pArg - arg2 >= sizeof(arg2) - 1 )
{
    send_to_char( "Password argument too long.\n\r", ch );
    return;
}
*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( ch->pcdata == NULL ||
         !password_matches( arg1, ch->pcdata->pwd,
                            !IS_EXTRA(ch,EXTRA_NEWPASS) ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    if ( !password_set(ch, arg2) )
    {
        send_to_char( "Password encryption failed. Try again.\n\r", ch );
        return;
    }
    do_autosave(ch,"");
    if (ch->desc != NULL && ch->desc->connected == CON_PLAYING )
	send_to_char( "Ok.\n\r", ch );
    return;
}

/*
 * Macintosh support functions.
 */
#if defined(macintosh)
long gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void mud_logf (char * fmt, ...)
{
        char buf [2*MAX_STRING_LENGTH];
        va_list args;
        va_start (args, fmt);
        vsnprintf (buf, sizeof(buf), fmt, args);
        va_end (args);

        log_string (buf);
}
