/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * Mainline
 * (c) 1995 Gavin J. Hurlbut
 * gjhurlbu@beirdo.ott.uplink.on.ca
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

static char     rcsid[] =
    "$Id$";

/*
 * Included Header Files 
 */
#ifndef CURSHEAD
#include <ncurses.h>
#else				/*
				 * def(CURSHEAD) 
				 */
#include CURSHEAD
#endif				/*
				 * CURSHEAD 
				 */

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

/*
 * Global Variables 
 */
int             numeric,
                quit,
                remotemode;
int             trunc_local_hosts = 0;
char           *revision;
char           *remoteip;
char           *curr_state;

#ifdef USE_OBSTACKS
struct obstack  the_obstack;
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
#endif				/*
				 * USE_OBSTACKS 
				 */

static void
handlesig(int signum)
{
    fprintf(stderr, "Received signal %d - aborting\n\n", signum);
    exit(0);
}

int
main(int argc, char **argv)
{
    int             ret;
    char            rev[] =
	"Revision: " DIALD_TOP_RELEASE DIALD_TOP_PATCHLEVEL;
    extern FILE    *yyin;
    fd_set          fd_ctl;
    int             fd_yyin;

#ifdef USE_OBSTACKS
    obstack_init(&the_obstack);
#endif				/*
				 * USE_OBSTACKS 
				 */

    revision = &rev[0];

    read_command_line(argc, argv);

    curr_state = (char *) malloc(LINELENGTH);

    screen_init();
    atexit(close_screen);

    init_fifo();
    atexit(close_fifo);

    quit = 0;
    FD_ZERO(&fd_ctl);
    FD_SET(0, &fd_ctl);
    FD_SET(fd_yyin = fileno(yyin), &fd_ctl);

    signal(SIGTERM, handlesig);
    signal(SIGINT, handlesig);
    signal(SIGHUP, handlesig);
    signal(SIGPIPE, SIG_IGN);

    while (!quit) {
	fd_set          in,
	                err;

	in = err = fd_ctl;
	if (select(fd_yyin + 1, &in, NULL, &err, NULL) == -1) {
	    continue;
	}

	if (FD_ISSET(fd_yyin, &err) || FD_ISSET(0, &err)) {
	    quit = 1;
	    continue;
	}

	if (FD_ISSET(0, &in)) {
	    parse_keyboard();
	}

	if (FD_ISSET(fd_yyin, &in)) {
	    if ((ret = read_fifo()) == -1)
		break;

	    if (ret == 1)
		update_screen();
	}
    }
}

int
read_command_line(int argc, char **argv)
{
    extern char    *optarg;
    int             opt;

    input_file = stderr;
    monitor = strdup(tmpnam(NULL));
    control = strdup(CONTROL_FIFO);

    numeric = 0;
    remotemode = 0;

    while ((opt = getopt(argc, argv, "qlnhc:m:t:r:L:d")) != -1) {
	switch (opt) {
	case 'q':
	    quit_if_parse_error = 1;
	    break;

	case 'd':
	    yydebug = 1;
	    break;

	case 'n':
	    numeric = 1;
	    break;

	case 'c':
	    free(control);
	    control = strdup(optarg);
	    break;

	case 'r':
	    remoteip = strdup(optarg);
	    remotemode = 1;
	    break;

	case 'L':
	    if (!(input_file = fopen(optarg, "w"))) {
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
	    display_usage(argc, argv);
	    break;
	}
    }
    return (0);
}
