/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * Monitor Pipe Handling
 * (c) 1995-2002 Gavin J. Hurlbut
 * gjhurlbu@beirdo.ca
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "defines.h"
#include "prototypes.h"
#include "externs.h"

/*
 * Global Variables 
 */
char           *control;
char           *monitor;
char           *cmd[] =
{
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
int             count,
                queue,
                state;
char           *format;
int             monitorfd,
                controlfd;

extern int      yyparse(void);
extern FILE    *yyin;
extern int      yacc_eof;
extern int      yacc_block;

void
validate_command(char *command)
{
    int             i,
                    len;

    for (i = 0; cmd[i] != NULL; i++) {
	len = strlen(cmd[i]);
	if (!strncasecmp(cmd[i], strstrip(command), len))
	    break;
    }

    if (cmd[i] == NULL)
	command[0] = '\0';
    else
	strcpy(command, cmd[i]);
}

int
fifo_command(char *command)
{
    char           *line;
    int             len;

    len = strlen(command);

    /*
     * Ignore NULL commands 
     */
    if (len == 0)
	return (1);

    line = (char *) alloca(len + 2);

    sprintf(line, "%s\n", command);
    write(controlfd, line, strlen(line));
    return (0);
}

char           *
strstrip(char *line)
{
    int             len,
                    i;

    len = strlen(line);
    for (i = 0; i < len && !isspace(line[i]); i++);
    line[i] = '\0';

    return (line);
}

void
init_fifo(void)
{
    long            fifo_ctl;
    char           *line;
    fd_set          rdfd;
    struct timeval  tv;
    int             res;

    line = (char *) alloca(LINELENGTH);

    yacc_eof = 0;
    count = 0;

    if (remotemode) {
	struct sockaddr_in sa;
	struct in_addr  remaddr;

	if (!
#if HAVE_INET_PTON
	inet_pton(AF_INET, remoteip, &remaddr)
#else
	inet_aton(remoteip, &remaddr)
#endif
	) {
	    fprintf(stderr, "Invalid remote IP: %s\n", remoteip);
	    exit(1);
	}
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = 0;
	sa.sin_port = 0;

	monitorfd = socket(AF_INET, SOCK_STREAM, 6);
	if (bind(monitorfd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
	    fprintf(stderr, "Couldn't bind to socket\n");
	    exit(1);
	}
	sa.sin_addr.s_addr = remaddr.s_addr;
	sa.sin_port = htons(1313);
	if (connect(monitorfd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
	    fprintf(stderr, "Couldn't connect to %s:1313\n", remoteip);
	    exit(1);
	}
	fifo_ctl = fcntl(monitorfd, F_GETFL);
	fifo_ctl |= O_NONBLOCK;
	fcntl(monitorfd, F_SETFL, fifo_ctl);

	controlfd = dup(monitorfd);
	sprintf(line, "monitor on");

    } else {
	if ((controlfd = open(control, O_WRONLY | O_APPEND)) < 0) {
	    perror("Could not open diald control pipe");
	    exit(1);
	}
	umask(0002);
	mknod(monitor, S_IFIFO | 0660, 0);

	/*
	 * * Open the monitor pipe non-blocking mode initially * *
	 * Apparently, diald-0.16 expects a reader on the fifo *before* * it
	 * received the "monitor" command.  This will open the fifo, * and
	 * then submit the command.  After the command has been sent, * the
	 * fifo is reopened in the normal blocking mode. * * Patch from
	 * diald-top-1.0 provided by: * Steffen Ullrich <ccrlphr@xensei.com>
	 * * at Wed, 6 Aug 1997 20:40:42 +0200 * * and incorporated into
	 * diald-top-1.1 * at   Fri Aug 29 20:19:14 EDT 1997 
	 */

	if ((monitorfd = open(monitor, O_RDONLY | O_NONBLOCK)) < 0) {
	    perror("Could not open diald monitor pipe");
	    exit(1);
	}
	/*
	 * itz Sun Jan 17 13:36:07 PST 1999: don't overload the fifo with
	 * useless junk 
	 *
	 * gjh Wed Jun 21 21:48:39 EDT 2000: I think that this may not work
	 * precisely right with diald 0.99.4, so I am going to reverse it.
	 */
	sprintf(line, "monitor %s", monitor);

#if 0				/*
				 * Was itz's patch 
				 */
	sprintf(line, "monitor %d %s", 0x3b, monitor);
#endif
    }

    fifo_command(line);

    if (!remotemode) {
	/*
	 * Wait for activity on the monitor FIFO to ensure diald is
	 * running.   If no activity within 5 seconds, abort.
	 * Diald is likely screwed.  This timeout may be lengthened.
	 *
	 * Idea originally from:
	 *      Ian T Zimmerman <itz@rahul.net>
	 */

	FD_ZERO(&rdfd);
	FD_SET(monitorfd, &rdfd);

	tv.tv_sec = 5;
	tv.tv_usec = 0;

	res = select(monitorfd + 1, &rdfd, NULL, NULL, &tv);
	if (res <= 0) {
	    fprintf(stderr, "No activity on monitor pipe for 5 "
		    "sec.  Aborting!\n");
	    exit(1);
	}
    }
    if ((yyin = fdopen(monitorfd, "r")) == NULL) {
	fprintf(stderr, "Could not fdopen diald monitor pipe\n");
	exit(1);
    }
    format = (char *) malloc(80);
    sprintf(format, "%%-4.4s %%%d.%ds/%%-8.8s %%%d.%ds/%%-8.8s  %%s\n",
	    ip_width, ip_width, ip_width, ip_width);
}

int
strtcmp(char *str1, char *str2)
{
    return (strncmp(str1, str2, strlen(str2)));
}

int
read_fifo(void)
{
    int             i;

    yacc_eof = 0;
    i = 0;
    do {
	yyparse();
	i++;
    } while (!yacc_block && i <= 3);

    if (yacc_eof) {
	fprintf(stderr, "diald-top: end of file on monitor pipe\n");
	return (-1);
    }
    /*
     * Redisplay the packet list 
     */
    if (resize_event_p) {
      do_screen_resize();
      resize_event_p = 0;
    }
    do_state(curr_state);

    werase(sub);
    if (count == 0) {
	mvwprintw(sub, 0, 5, "No packets queued\n");
    } else {
	for (i = 0; i < numbuff && i < count; i++) {
	    mvwprintw(sub, i, 0, "%s", scrbuff[i]);
	}
    }

    return (1);
}

void
close_fifo(void)
{
    close(controlfd);
    fclose(yyin);
    unlink(monitor);
}
