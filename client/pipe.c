/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * (c) 1995 Gavin J. Hurlbut
 * gjhurlbu@beirdo.uplink.on.ca
 *
 * $Log$
 * Revision 2.0  1997/09/28 21:21:13  gjhurlbu
 * Release 2.0
 *
 * Revision 1.16  1997/09/28 01:07:24  gjhurlbu
 * Changed to always use nonblocked I/O on the monitor FIFO
 * Allows multiple redraws to be combined
 * Fixed help screen - now will use 2 screens if not enough lines to do it in 1
 * Changed usage information (diald-top -h) to use one string for the entire
 * display, and put it on stdout rather than stderr to allow {less,more} to grok it
 *
 * Revision 1.15  1997/09/27 05:53:56  gjhurlbu
 * Fixed non-initialization of curr_state
 *
 * Revision 1.14  1997/09/25 06:45:22  gjhurlbu
 * Tidied the client code some
 * Converted some static buffers to dynamically created ones
 *
 * Revision 1.13  1997/09/25 03:46:50  gjhurlbu
 * Nearly ready for release
 * Added patches from Steffen Ullrich
 *
 * Revision 1.12  1997/09/02 02:14:36  gjhurlbu
 * Created a substructure
 *
 * Revision 1.11  1997/08/30 19:54:14  gjhurlbu
 * Incorporated many patches
 * Fixed redraw
 *
 * Revision 1.10  1997/08/30 00:23:36  gjhurlbu
 * Added patch from:	Steffen Ullrich <ccrlphr@xensei.com>
 * received		Wed, 6 Aug 1997 20:40:42 +0200
 *
 * Opens monitor fifo in nonblocking mode initially, and reopens in blocking
 * mode after submitting the monitor command to diald.
 *
 * Revision 1.9  1997/03/30 04:10:31  gjhurlbu
 * Stupid version screwups
 *
 * Revision 1.8  1997/03/29 17:34:37  gjhurlbu
 * Added RCS IDs to all C files
 * Removed some excess comments
 *
 * Revision 1.7  1997/01/15 07:46:30  gjhurlbu
 * Changed to buffer per line so seg faults disappear.
 * Changed usleep to 999.900 ms for some sort of CPU niceness
 *
 * Revision 1.6  1996/07/28 07:52:13  gjhurlbu
 * Expanded to use the entire width of the screen
 *
 * Revision 1.5  1996/07/28 06:27:38  gjhurlbu
 * Should be more processor friendly
 * Still to do: take advantage of FULL width of the screen rather than sticking to 80 characters
 *
 */

static char rcsid[] = "$Id$";

/* Included Header Files */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "defines.h"
#include "prototypes.h"
#include "externs.h"

/* Global Variables */
char *control;
char *monitor;
char *cmd[] = {
	"up",
	"down",
	"force",
	"unforce",
	"block",
	"unblock",
	"quit",
	"delayed-quit",
	"reset",
	NULL
};
int count, queue, state;
char *format;
int monitorfd, controlfd;

extern int yyparse( void );
extern FILE *yyin;
extern int yacc_eof;
extern int yacc_block;

void validate_command(char *command)
{
	int i, len;

	for( i = 0; cmd[i] != NULL; i++ ) {
		len = strlen( cmd[i] );
		if ( !strncasecmp( cmd[i], strstrip(command), len ) ) break;
	}

	if( cmd[i] == NULL )	command[0] = '\0';
	else			strcpy( command, cmd[i] );
}


int fifo_command(char *command)
{
	char *line;
	int len;

	len = strlen( command );

	/* Ignore NULL commands */
	if( len == 0 )		return( 1 );

	line = (char *)alloca(len+2);

	sprintf( line, "%s\n", command );
	write( controlfd, line, strlen( line ) );
	return( 0 );
}


char *strstrip(char *line)
{
	int len, i;

	len = strlen(line);
	for( i = 0; i < len && !isspace(line[i]); i++ );
	line[i] = '\0';

	return( line );
}

void init_fifo( void ) {
	long	fifo_ctl;
	char	*line;
	fd_set	rdfd;
	struct timeval tv;
	int 	res;

	line = (char *)alloca(LINELENGTH);

	yacc_eof = 0;
	count = 0;

	if( remotemode ) {
		struct sockaddr_in sa;
		struct in_addr remaddr;

		if( !inet_aton( remoteip, &remaddr ) ) {
			fprintf( stderr, "Invalid remote IP: %s\n", remoteip );
			exit(1);
		}
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = 0;
		sa.sin_port = 0;

		monitorfd = socket( AF_INET, SOCK_STREAM, 6 );
		if( bind( monitorfd, (struct sockaddr *)&sa, sizeof(sa)) < 0 ) {
			fprintf( stderr, "Couldn't bind to socket\n" );
			exit(1);
		}

		sa.sin_addr.s_addr = remaddr.s_addr;
		sa.sin_port = htons( 1313 );
		if( connect(monitorfd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
			fprintf( stderr, "Couldn't connect to %s:1313\n",
				 remoteip );
			exit(1);
		}

		fifo_ctl = fcntl( monitorfd, F_GETFL );
		fifo_ctl |= O_NONBLOCK;
		fcntl( monitorfd, F_SETFL, fifo_ctl );

		controlfd = dup( monitorfd );
		sprintf(line,"monitor on");
		
	} else {
		if ( (controlfd = open(control, O_WRONLY | O_APPEND) ) < 0 ) {
			perror("Could not open diald control pipe");
			exit(1);
		}

		umask( 0002 );
		mknod( monitor, S_IFIFO | 0660, 0 );


		/*
	 	* Open the monitor pipe non-blocking mode initially
	 	*
	 	* Apparently, diald-0.16 expects a reader on the fifo *before*
		* it received the "monitor" command.  This will open the fifo,
		* and then submit the command.  After the command has been sent,
		* the fifo is reopened in the normal blocking mode.
	 	*
	 	* Patch from diald-top-1.0 provided by:
	 	*	Steffen Ullrich <ccrlphr@xensei.com>
	 	* at	Wed, 6 Aug 1997 20:40:42 +0200
	 	*
	 	* and incorporated into diald-top-1.1
	 	* at	Fri Aug 29 20:19:14 EDT 1997
	 	*/


		if ( (monitorfd = open(monitor, O_RDONLY | O_NONBLOCK) ) < 0 ) {
			perror("Could not open diald monitor pipe");
			exit(1);
		}
		sprintf(line,"monitor %s",monitor);
	}

	fifo_command(line);
        

	if( !remotemode ) {
		/*
	 	 * Wait for activity on the monitor FIFO to ensure diald is
		 * running.   If no activity within 5 seconds, abort.
		 * Diald is likely screwed.  This timeout may be lengthened.
	 	 *
	 	 * Idea originally from:
	 	 * 	Ian T Zimmerman <itz@rahul.net>
	 	 */

		FD_ZERO( &rdfd );
		FD_SET( monitorfd, &rdfd );

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		res = select( monitorfd+1, &rdfd, NULL, NULL, &tv );
		if( res <= 0 ) {
			fprintf( stderr, "No activity on monitor pipe for 5 "
					 "sec.  Aborting!\n");
			exit(1);
		}
	}

	if( (yyin = fdopen(monitorfd,"r") ) == NULL ) {
		fprintf( stderr, "Could not fdopen diald monitor pipe\n" );
		exit( 1 );
	}

	format = (char *)malloc(80);
	sprintf( format, "%%-4.4s %%%d.%ds/%%-8.8s %%%d.%ds/%%-8.8s  %%s\n", 
		ip_width, ip_width, ip_width, ip_width );
}

int strtcmp( char *str1, char *str2 ) {
	return( strncmp( str1, str2, strlen(str2) ) );
}

int read_fifo( void ) {
	int  i;

        yacc_eof = 0;
	i = 0;
	do {
		yyparse();
		i++;
	} while( !yacc_block && i <= 3 );

	if ( yacc_eof ) {
          fprintf (stderr, "diald-top: end of file on monitor pipe\n");
          return( -1 );
        }

	/* Redisplay the packet list */
	do_state( curr_state );

	werase( sub );
	if ( count == 0 ) {
		mvwprintw(sub,0,5,"No packets queued\n");
	} else {
		wmove( sub, 0, 0 );
		for( i = 0; i < numbuff && i < count ; i++ ) {
			wprintw(sub, "%s", scrbuff[i] );
		}
	}

	return( 1 );
}

void close_fifo( void ) {
	close( controlfd );
	fclose( yyin );
	unlink( monitor );
}
