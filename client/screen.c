/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * Ncurses Screen Control
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

/*
 * Global Variables 
 */
WINDOW         *full,
               *sub;
char           *scrbuff[MAXBUFS];
int             numbuff;
int             ip_width;
static char    *usagetext = "\n"
    "DialD Packet statistics program (c) 1995-2001 Gavin J. Hurlbut\n"
    "%s \thttp://diald-top.sourceforge.net/\n"
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
    "The -r option allows you to connect to a diald-top-server over the "
    "network\n"
    "  using a TCP connection to port 1313 on the machine at IP address "
    "provided.\n"
    "\n"
    "The -L option creates a log file with all monitor FIFO input logged with "
    "a\n"
    "  timestamp to aid in debugging.  This is likely to create a very large "
    "file\n" "  if left running!\n" "\n";

void
get_command(char *command)
{
    mvwprintw(full, LINE_COMMAND, 0, "FIFO Command:");
    update_screen();

    curs_set(1);
    echo();
    mvwgetstr(full, LINE_COMMAND, 15, command);
    noecho();
    curs_set(0);
}

void
get_timeout(char *line)
{
    mvwprintw(full, LINE_COMMAND, 0, "Timeout (in us):");
    update_screen();

    curs_set(1);
    echo();
    mvwgetstr(full, LINE_COMMAND, 18, line);
    noecho();
    curs_set(0);
}

void
helpscreen(void)
{
    int             rowsavail = LINES - HEAD_LINES;

    print_help(0, NULL);
    print_help(0, "(c)\tSend a command to the diald control FIFO");

    if (rowsavail >= 17) {
	show_help_commands();
    }

    print_help(0, "(h)\tDisplay Help Screen (this screen)");
    print_help(0,
	       "(l)\tToggle Truncation of Local Hosts (removes domain)");
    print_help(0, "(n)\tToggle Numeric IP vs. Resolved Names");
    print_help(0, "(q)\tQuit diald-top");
    print_help(0, "(^R)\tRedraw screen");

    print_help(0, "");

    if (rowsavail >= 17) {
	print_help(26, "Press any key to resume");
    } else {
	print_help(25, "Press any key to continue");
    }

    update_screen();
    while (getch() == ERR);

    if (rowsavail < 17) {
	print_help(0, NULL);
	show_help_commands();

	print_help(0, "");
	print_help(26, "Press any key to resume");

	update_screen();
	while (getch() == ERR);
    }

}

void
show_help_commands(void)
{
    print_help(0, "Valid Commands:");
    print_help(10, "up           Bring the link up");
    print_help(10, "down         Bring the link down");
    print_help(10, "force        Force the link up until further notice");
    print_help(10, "unforce      No longer force the link up");
    print_help(10,
	       "block        Force the link down until further notice");
    print_help(10, "unblock      No longer force the link down");
    print_help(10, "quit         Forcibly quit diald (and shut down the "
	       "link)");
    print_help(10, "delay-quit   Quit diald the next time the link comes "
	       "down");
}

void
print_help(int x, char *line)
{
    static int      currline;

    if (!line) {
	werase(sub);
	currline = 0;
	return;
    }
    mvwprintw(sub, ++currline, x, line);
}

void
display_usage(int argc, char **argv)
{
    printf(usagetext, revision, argv[0], CONTROL_FIFO);
    exit(2);
}

void
screen_init(void)
{
    char           *fmt;
    int             spcs;
    int             lines,
                    cols;

    fmt = (char *) alloca(80);

    initscr();
    full = newwin(0, 0, 0, 0);
    sub = subwin(full, LINES - HEAD_LINES, COLS, HEAD_LINES, 0);
    curs_set(0);
    nodelay(stdscr, TRUE);
    cbreak();
    noecho();

    lines = LINES;
    cols = COLS;
    for (numbuff = 0; numbuff <= lines; numbuff++) {
	scrbuff[numbuff] = (char *) malloc(cols);
    }

    strcpy(curr_state, "");
    werase(full);

    spcs = COLS - 61;
    if (spcs < 0)
	spcs = 0;
    sprintf(fmt, "%%-s%%%ds%%-s", spcs);
    mvwprintw(full, LINE_COPYRIGHT, 0, fmt,
	      "DialD Packet statistics program", "",
	      "(c) 1995-2001 Gavin J. Hurlbut");

    spcs = (COLS - 45 - strlen(revision)) / 2;
    sprintf(fmt, "%%-s%%%ds%%-s%%%ds%%-s", spcs, spcs);
    mvwprintw(full, LINE_REVISION, 0, fmt, revision, "",
	      "http://diald-top.sourceforge.net/", "", "(h) for help");

    ip_width = (COLS - 36) / 2;
    sprintf(fmt, "%%-4.4s %%%ds          %%%ds           %%s", ip_width,
	    ip_width);
    mvwprintw(full, LINE_HEADERS, 0, fmt, "Type", "Destination", "Source",
	      "TTL");
    mvwprintw(full, LINE_UNDERLINE, 0, "%s", strrpt('=', COLS - 1));

    update_screen();
}

char           *
strrpt(char ch, int count)
{
    char           *string;
    int             i;

    string = (char *) malloc(count + 1);

    for (i = 0; i < count; i++)
	string[i] = ch;

    string[i] = '\0';
    return (string);
}

void
update_screen(void)
{
    touchwin(full);
    wmove(full, LINE_COMMAND, 0);
    wrefresh(full);
}

void
close_screen(void)
{
    nodelay(stdscr, FALSE);
    nocbreak();
    curs_set(1);
    delwin(sub);
    delwin(full);
    endwin();
}

void
parse_keyboard(void)
{
    char            key;
    char           *line;
    unsigned long int tim;

    line = (char *) alloca(LINELENGTH);
    key = getch();

    switch (key) {
    case 'd':
	yydebug = !yydebug;
	mvwprintw(full, LINE_COMMAND, 0, "Debug mode is %s",
		  (yydebug ? "ON " : "OFF"));
	update_screen();
	while (getch() == ERR);

	wmove(full, LINE_COMMAND, 0);
	wclrtoeol(full);
	update_screen();
	break;

    case 'q':
	quit = 1;
	break;

    case 'h':
	helpscreen();
	wclear(sub);
	update_screen();
	break;

    case 'c':
	get_command(line);
	validate_command(line);
	fifo_command(line);
	wmove(full, LINE_COMMAND, 0);
	wclrtoeol(full);
	update_screen();
	break;

    case 'n':
	numeric = 1 - numeric;
	break;

    case 'l':
	trunc_local_hosts = 1 - trunc_local_hosts;
	break;

    case 18:			/*
				 * ^R 
				 */
	clearok(full, TRUE);
	update_screen();
	break;
    }
}
