/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * (c) 1995 Gavin J. Hurlbut
 * gjhurlbu@beirdo.uplink.on.ca
 *
 * $Log$
 * Revision 2.0  1997/09/28 21:21:16  gjhurlbu
 * Release 2.0
 *
 * Revision 1.17  1997/09/28 21:01:59  gjhurlbu
 * Added -L option to usage info and man page
 *
 * Revision 1.16  1997/09/28 05:22:12  gjhurlbu
 * Changed to use only nonblocking I/O, but not to quit immediately on a EAGAIN
 * Now waits to 500ms for input before giving up
 * Seems to work fine over the network (even over the modem)
 * Added parsing rules to catch partial packet QUEUEs without crapping out
 *
 * Revision 1.15  1997/09/28 01:07:26  gjhurlbu
 * Changed to always use nonblocked I/O on the monitor FIFO
 * Allows multiple redraws to be combined
 * Fixed help screen - now will use 2 screens if not enough lines to do it in 1
 * Changed usage information (diald-top -h) to use one string for the entire
 * display, and put it on stdout rather than stderr to allow {less,more} to grok it
 *
 * Revision 1.14  1997/09/25 06:45:24  gjhurlbu
 * Tidied the client code some
 * Converted some static buffers to dynamically created ones
 *
 * Revision 1.13  1997/09/02 04:43:58  gjhurlbu
 * Added the -r to the usage
 *
 * Revision 1.12  1997/09/02 03:49:55  gjhurlbu
 * Much better.  I think the Makefiles are pretty well ready...
 * Now to fix the documentation.
 *
 * Revision 1.11  1997/08/30 19:54:16  gjhurlbu
 * Incorporated many patches
 * Fixed redraw
 *
 * Revision 1.10  1997/03/30 04:10:32  gjhurlbu
 * Stupid version screwups
 *
 * Revision 1.9  1997/03/29 17:34:38  gjhurlbu
 * Added RCS IDs to all C files
 * Removed some excess comments
 *
 * Revision 1.8  1997/03/29 07:07:53  gjhurlbu
 * Added adjustable timeouts
 * Starting to build up towards using lex/yacc to parse the contents of the
 * monitor file
 * Next release (with lex/yacc) will be 1.0
 *
 * Revision 1.7  1997/01/15 07:46:31  gjhurlbu
 * Changed to buffer per line so seg faults disappear.
 * Changed usleep to 999.900 ms for some sort of CPU niceness
 *
 * Revision 1.6  1996/07/28 08:05:26  gjhurlbu
 * Opps
 *
 * Revision 1.5  1996/07/28 08:02:05  gjhurlbu
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
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <malloc.h>

#include "defines.h"
#include "prototypes.h"
#include "externs.h"


/* Global Variables */
WINDOW	*full, *sub;
char	*scrbuff[MAXBUFS];
int	numbuff;
int	ip_width;

void get_command( char *command )
{
	mvwprintw( full, LINE_COMMAND, 0, "FIFO Command:" );
	update_screen();

	curs_set( 1 );
	echo();
	mvwgetstr( full, LINE_COMMAND, 15, command );
	noecho();
	curs_set( 0 );
}

void get_timeout( char *line )
{
	mvwprintw( full, LINE_COMMAND, 0, "Timeout (in us):" );
	update_screen();

	curs_set( 1 );
	echo();
	mvwgetstr( full, LINE_COMMAND, 18, line );
	noecho();
	curs_set( 0 );
}

void helpscreen( void )
{
	int rowsavail = LINES - HEAD_LINES;

	print_help( 0, NULL );
	print_help( 0, "(c)\tSend a command to the diald control FIFO" );

	if( rowsavail >= 17 ) {
		show_help_commands();
	}

	print_help( 0, "(h)\tDisplay Help Screen (this screen)" );
	print_help( 0, "(l)\tToggle Truncation of Local Hosts (removes "
			"domain)" );
	print_help( 0, "(n)\tToggle Numeric IP vs. Resolved Names" );
	print_help( 0, "(q)\tQuit diald-top" );
	print_help( 0, "(^R)\tRedraw screen" );

	print_help( 0, "" );

	if( rowsavail >= 17 ) {
		print_help( 26, "Press any key to resume" );
	} else {
		print_help( 25, "Press any key to continue" );
	}

	update_screen();
	while( getch() == ERR );

	if( rowsavail < 17 ) {
		print_help( 0, NULL );
		show_help_commands();

		print_help( 0, "" );
		print_help( 26, "Press any key to resume" );

		update_screen();
		while( getch() == ERR );
	}
		
}

void show_help_commands( void )
{
	print_help( 0, "Valid Commands:" );
	print_help( 10, "up           Bring the link up" );
	print_help( 10, "down         Bring the link down" );
	print_help( 10, "force        Force the link up until further notice" );
	print_help( 10, "unforce      No longer force the link up" );
	print_help( 10, "block        Force the link down until further "
			"notice" );
	print_help( 10, "unblock      No longer force the link down" );
	print_help( 10, "quit         Forcibly quit diald (and shut down the "
			"link)" );
	print_help( 10, "delay-quit   Quit diald the next time the link comes "
			"down" );
}

void print_help( int x, char * line ) 
{
	static int currline;

	if( !line ) {
		werase( sub );
		currline = 0;
		return;
	}
	mvwprintw( sub,  ++currline,  x, line );
}

void display_usage( int argc, char **argv )
{
	char *usagetext = "\n"
"DialD Packet statistics program (c) 1995-97 Gavin J. Hurlbut\n"
"%s \tgjhurlbu@beirdo.ott.uplink.on.ca\n"
"\n"
"Proper usage:\n"
"\n"
"\t%s [-n] [-l] [-q] [-c control_FIFO] [-r address] [-L logfile]\n"
"\n"
"If the -n option is used, the IP addresses will not be resolved\n"
"  otherwise, all IP addresses will be resolved (when the link is up)\n"
"\n"
"If the -l option is used, any hosts in the local domain will have\n"
"  the domain name stripped off, otherwise, the domain name will be\n"
"  included.\n"
"\n"
"If the -q option is used, diald-top will quit on a parse error\n"
"\n"
"The -c option is optional.  If it is not included, the control FIFO\n"
"  will default to: %s\n"
"\n"
"The -r option allows you to connect to a diald-top-server over the network\n"
"  using a TCP connection to port 1313 on the machine at IP address provided.\n"
"\n"
"The -L option creates a log file with all monitor FIFO input logged with a\n"
"  timestamp to aid in debugging.  This is likely to create a very large file\n"
"  if left running!\n"
"\n";
	

	printf( usagetext, revision, argv[0], CONTROL_FIFO );
	exit( 2 );
}

void screen_init( void )
{
	char	*fmt;
	int	spcs;
	int	lines, cols;

	fmt = (char *)alloca(80);

	initscr();
	full = newwin( 0, 0, 0, 0 );
	sub  = subwin( full, LINES-HEAD_LINES, COLS, HEAD_LINES, 0 );
	curs_set( 0 );
	nodelay( stdscr, TRUE );
	cbreak();
	noecho();

	lines = LINES;
	cols  = COLS;
	for( numbuff = 0; numbuff <= lines; numbuff++ ) {
		scrbuff[numbuff] = (char *)malloc(cols);
	}

	strcpy( curr_state, "" );
	werase( full );

	spcs = COLS - 61;
	if( spcs < 0 )	spcs = 0;
	sprintf( fmt, "%%-s%%%ds%%-s", spcs );	
	mvwprintw( full, LINE_COPYRIGHT, 0, fmt,
			"DialD Packet statistics program", "",
			"(c) 1995-97 Gavin J. Hurlbut" );

	spcs = (COLS - 45 - strlen( revision ))/2;
	sprintf( fmt, "%%-s%%%ds%%-s%%%ds%%-s", spcs, spcs );
	mvwprintw(full, LINE_REVISION, 0, fmt, revision, "",
			"gjhurlbu@beirdo.ott.uplink.on.ca",
			"", "(h) for help" );

	ip_width = (COLS-36) / 2;
	sprintf( fmt, "%%-4.4s %%%ds          %%%ds           %%s", ip_width,
			ip_width );
	mvwprintw( full, LINE_HEADERS, 0, fmt, "Type", "Destination", "Source",
			"TTL" );
	mvwprintw( full, LINE_UNDERLINE, 0, "%s", strrpt( '=', COLS-1 ) );

	update_screen();
}


char *strrpt( char ch, int count )
{
	char *string;
	int i;

	string = (char *)malloc(count+1);

	for( i = 0; i < count; i++ )
		string[i] = ch;

	string[i] = '\0';
	return( string );
}

void update_screen( void )
{
	touchwin( full ); 
	wmove( full, LINE_COMMAND, 0 );
	wrefresh( full );
}

void close_screen( void )
{
	nodelay( stdscr, FALSE );
	nocbreak();
	curs_set( 1 );
	delwin( sub );
	delwin( full );
	endwin();
}

void parse_keyboard( void )
{
	char key;
	char *line;
	unsigned long int tim;

	line = (char *)alloca(LINELENGTH);
	key = getch();

	switch( key ) 
	{
        case 'd':
                yydebug = !yydebug;
                mvwprintw(full, LINE_COMMAND, 0, "Debug mode is %s",
				(yydebug ?  "ON " : "OFF") );
		update_screen();
		while( getch() == ERR );

		wmove( full, LINE_COMMAND, 0 );
		wclrtoeol( full );
		update_screen();
                break;

	case 'q':
		quit = 1;
		break;

	case 'h':
		helpscreen();
		wclear( sub );
		update_screen();
		break;

	case 'c':
		get_command( line );
		validate_command( line );
		fifo_command( line );
		wmove( full, LINE_COMMAND, 0 );
		wclrtoeol( full );
		update_screen();
		break;

	case 'n':
		numeric = 1 - numeric;
		break;

	case 'l':
		trunc_local_hosts = 1 - trunc_local_hosts;
		break;

	case 18:		/* ^R */
		clearok( full, TRUE );
		update_screen();
		break;
	}
}