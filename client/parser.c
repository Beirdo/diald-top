/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * Pipe contents Parser (for testing)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "parser.h"
#include "externs.h"

int             rxload[3] = { 0, 0, 0 };
int             txload[3] = { 0, 0, 0 };
time_t          now;

void
do_status(int up, int force, int im, int im_itm, int im_tm, int im_fuzz,
	  char *im_to, char *force_to, char *to)
{
#ifdef TESTER
    printf("Status:  up = %d; force = %d; im = %d; im_itm = %d; "
	   "im_tm = %d;\n\tim_fuzz = %d; im_to = %s; force_to = %s; "
	   "to = %s\n", up, force, im, im_itm, im_tm, im_fuzz, im_to,
	   force_to, to);
#endif
}

void
do_load(int itxtotal, int irxtotal)
{
#ifdef TESTER
    printf("Load: Tx = %d; Rx = %d\n", itxtotal, irxtotal);
#else
    rxload[0] = rxload[1];
    txload[0] = txload[1];
    rxload[1] = rxload[2];
    txload[1] = txload[2];
    rxload[2] = irxtotal;
    txload[2] = itxtotal;
#endif
}

void
do_interface(char *interface, char *local_ip, char *remote_ip)
{
#ifdef TESTER
    printf("Interface: IF = %s; Local IP = %s; Remote IP = %s\n",
	   interface, local_ip, remote_ip);
#else
    int             spcs;
    char           *fmt,
                   *line;

    fmt = alloca(COLS);
    line = alloca(COLS + 2);

    wmove(full, LINE_INTERFACE, 0);
    wclrtoeol(full);

    spcs = (COLS - 32 - strlen(interface) - strlen(remote_ip)
	    - strlen(local_ip)) / 2;
    sprintf(fmt, "%%-s%%s%%%ds%%-s%%s%%%ds%%-s%%s", spcs, spcs);
    sprintf(line, fmt, "Interface: ", interface, "", "Local IP: ",
	    local_ip, "", "Remote IP: ", remote_ip);
    mvwprintw(full, LINE_INTERFACE, 0, "%s", line);
#endif
}

void
do_state(char *state)
{
#ifdef TESTER
    printf("State:  %s\n", state);
#else
    char           *line;

    line = (char *) alloca(LINELENGTH);

    strcpy(curr_state, state);

    wmove(full, LINE_STATE, 0);
    wclrtoeol(full);

    sprintf(line, "Link Status:  %s          ", curr_state);
    mvwprintw(full, LINE_STATE, 0, "%s", line);

    sprintf(line, "   Packets Queued:  %d   ", count);
    mvwprintw(full, LINE_STATE, (COLS - strlen(line)) / 2, "%s", line);
#endif
}

void
do_message(char *message)
{
#ifdef TESTER
    printf("Message:  %s\n", message);
#else
    wmove(full, LINE_MESSAGE, 0);
    wclrtoeol(full);

    mvwprintw(full, LINE_MESSAGE, 0, "Message:  %s", message);
#endif
}

void
do_endqueue()
{
#ifdef TESTER
    printf("End Queue\n");
#endif
}

void
do_queue()
{
#ifdef TESTER
    printf("Start Queue\n");
#else
    int             i;

    now = time(0);
    count = 0;
    werase(sub);
    for (i = 0; i < numbuff; i++) {
	scrbuff[i][0] = '\0';
    }
#endif
}

void
do_packet(char *protocol, char *ip1, int port1, char *ip2, int port2,
	  char *ttl)
{
#ifdef TESTER
    printf("Packet: Protocol = %s; From = %s/%d; To = %s/%d; TTL = %s\n",
	   protocol, ip1, port1, ip2, port2, ttl);
#else
    char           *destports,
                   *sourceports;

    if (count <= LINES - HEAD_LINES) {
	destports = (char *) service(port1, protocol);
	sourceports = (char *) service(port2, protocol);

	sprintf(scrbuff[count], format, protocol, resolve(ip1),
		destports, resolve(ip2), sourceports, ttl);
	lex_free(sourceports);
	lex_free(destports);
    }

    count++;
#endif
}

int
lex_testkey(void)
{
#ifdef TESTER
    return (0);
#else
    int             ready;
    fd_set          keyset;
    struct timeval  delay;

    FD_ZERO(&keyset);
    FD_SET(0, &keyset);

    delay.tv_sec = 0;
    delay.tv_usec = 0;

    ready = select(1, &keyset, NULL, NULL, &delay);

    yacc_key = ready;
    return (yacc_key);
#endif
}
