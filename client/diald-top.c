/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * (c) 1995 Gavin J. Hurlbut
 * gjhurlbu@beirdo.uplink.on.ca
 *
 * $Log$
 * Revision 1.2  2000/06/12 13:56:18  gjhurlbu
 * Fixing Bug #107361 - input_file initialized with stderr which is not a constant in newer Linux systems.
 *
 * Revision 1.1.1.1  2000/06/11 05:57:08  gjhurlbu
 * Initial check-in from diald-top v2.0
 *
 *
 * Revision 2.0  1997/09/28 21:21:07  gjhurlbu
 * Release 2.0
 *
 * Revision 1.10  1997/09/28 05:22:08  gjhurlbu
 * Changed to use only nonblocking I/O, but not to quit immediately on a EAGAIN
 * Now waits to 500ms for input before giving up
 * Seems to work fine over the network (even over the modem)
 * Added parsing rules to catch partial packet QUEUEs without crapping out
 *
 * Revision 1.9  1997/09/28 01:07:19  gjhurlbu
 * Changed to always use nonblocked I/O on the monitor FIFO
 * Allows multiple redraws to be combined
 * Fixed help screen - now will use 2 screens if not enough lines to do it in 1
 * Changed usage information (diald-top -h) to use one string for the entire
 * display, and put it on stdout rather than stderr to allow {less,more} to grok it
 *
 * Revision 1.8  1997/09/27 06:31:23  gjhurlbu
 * Deal with keyboard input before monitor input after a select.  Seems to be more
 * responsive this way
 *
 * Revision 1.7  1997/09/27 05:53:55  gjhurlbu
 * Fixed non-initialization of curr_state
 *
 * Revision 1.6  1997/09/25 06:45:18  gjhurlbu
 * Tidied the client code some
 * Converted some static buffers to dynamically created ones
 *
 * Revision 1.5  1997/09/25 03:46:46  gjhurlbu
 * Nearly ready for release
 * Added patches from Steffen Ullrich
 *
 * Revision 1.4  1997/09/02 04:43:56  gjhurlbu
 * Added the -r to the usage
 *
 * Revision 1.3  1997/09/02 03:49:54  gjhurlbu
 * Much better.  I think the Makefiles are pretty well ready...
 * Now to fix the documentation.
 *
 * Revision 1.2  1997/09/02 02:14:34  gjhurlbu
 * Created a substructure
 *
 * Revision 1.1  1997/08/30 19:54:10  gjhurlbu
 * Incorporated many patches
 * Fixed redraw
 *
 * Revision 1.0  1997/03/30 04:08:55  gjhurlbu
 * I consider this ready for release once again
 * Uses Flex & Bison to do the parsing of the monitor pipe
 * Seems to be a little more CPU hungry than older versions, but allows for
 * easier expansion to use the "Load" and "State" parts of the monitor pipe
 *
 * Whatever.
 *
 * Revision 0.9  1997/03/29 17:34:36  gjhurlbu
 * Added RCS IDs to all C files
 * Removed some excess comments
 *
 * Revision 0.8  1997/03/29 07:07:49  gjhurlbu
 * Added adjustable timeouts
 * Starting to build up towards using lex/yacc to parse the contents of the
 * monitor file
 * Next release (with lex/yacc) will be 1.0
 *
 * Revision 0.7  1997/01/15 07:46:29  gjhurlbu
 * Changed to buffer per line so seg faults disappear.
 * Changed usleep to 999.900 ms for some sort of CPU niceness
 *
 * Revision 0.6  1996/07/28 08:02:03  gjhurlbu
 * Fixed usage info
 *
 */

static char rcsid[] = "$Id$";

/* Included Header Files */
#ifndef CURSHEAD
  #include <ncurses.h>
#else
  #include CURSHEAD
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "defines.h"
#include "prototypes.h"
#include "externs.h"
#include "version.h"


/* Global Variables */
int numeric, quit, remotemode;
int trunc_local_hosts = 0;
char *revision;
char *remoteip;
char *curr_state;

#ifdef USE_OBSTACKS
struct obstack the_obstack;
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
#endif /* USE_OBSTACKS */

static void handlesig( int signum )
{
	fprintf( stderr, "Received signal %d - aborting\n\n", signum );
	exit( 0 );
}

int main(int argc, char **argv)
{
	int ret;
	char rev[] = "Revision: " DIALD_TOP_RELEASE DIALD_TOP_PATCHLEVEL;
	extern FILE *yyin;
	fd_set fd_ctl;
	int fd_yyin;


#ifdef USE_OBSTACKS
        obstack_init(&the_obstack);
#endif /* USE_OBSTACKS */

	revision = &rev[0];

	read_command_line(argc,argv);

	curr_state = (char *)malloc(LINELENGTH);

	screen_init();
	atexit( close_screen );

	init_fifo();
	atexit( close_fifo );

	quit = 0;
	FD_ZERO( &fd_ctl );
	FD_SET( 0, &fd_ctl );
	FD_SET( fd_yyin = fileno( yyin ), &fd_ctl );

	signal( SIGTERM, handlesig );
	signal( SIGINT,  handlesig );
	signal( SIGHUP,  handlesig );
	signal( SIGPIPE, SIG_IGN );

	while ( !quit )
	{
		fd_set in, err;

		in = err = fd_ctl;
		if( select( fd_yyin+1, &in, NULL, &err, NULL ) == -1 ) {
			continue;
		}

		if( FD_ISSET( fd_yyin, &err ) || FD_ISSET( 0, &err ) ) {
			quit = 1;
			continue;
		}


		if( FD_ISSET( 0, &in ) ) {
			parse_keyboard();
		}

		if( FD_ISSET( fd_yyin, &in ) ) {
			if( (ret = read_fifo()) == -1 )
				break;

			if( ret == 1 )
				update_screen();
		}
	}
}



int read_command_line(int argc, char **argv)
{
	extern char *optarg;
	int opt;

	input_file = stderr;
	monitor = strdup( tmpnam(NULL) );
	control = strdup( CONTROL_FIFO );

	numeric=0;
	remotemode = 0;

	while ( (opt=getopt(argc,argv,"qlnhc:m:t:r:L:d")) != -1 )
	{
		switch(opt) {
                case 'q':
                        quit_if_parse_error = 1;
                        break;

                case 'd':
                        yydebug = 1;
                        break;

		case 'n':
			numeric=1;
			break;

		case 'c':
			free( control );
			control = strdup(optarg);
			break;

		case 'r':
			remoteip = strdup( optarg );
			remotemode = 1;
			break;


		case 'L':
			if (!(input_file = fopen(optarg,"w"))) {
				perror(optarg);
				exit(1);
			}
			yy_input_func = yy_log_input;
			break;

		case 'l':
			trunc_local_hosts = 1;
			break;

		case ':':
		case '?':
		case 'h':
			display_usage(argc,argv);
			break;
		}
	}
	return(0);
}

