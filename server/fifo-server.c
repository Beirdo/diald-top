/*
 * $Id$
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

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <syslog.h>
#include <errno.h>

#include "config.h"
#include "fds.h"
#include "ports.h"
#include "protos.h"
#include "loglevels.h"

static char     rcsid[] =
    "$Id$";

int             controlfd = -1;
int             monitorfd = -1;
int             monitorcount = 0;
char            monitor[255];
char            line[1002];

extern int      errno;

thread_fds_t   *
fifo_server_init(void)
{
    thread_fds_t   *fds;

    controlfd = open(CONTROL_FIFO, O_WRONLY | O_APPEND);

    fds = (thread_fds_t *) malloc(sizeof(thread_fds_t));

    fds->fullcount = 0;
    fds->selcount = 0;
    FD_ZERO(&fds->full);
    FD_ZERO(&fds->sel);
    fds->maxfd = -1;

    return (fds);
}

void
monitor_open(void)
{
    long            fifo_ctl;

    if (monitorfd != -1)
	return;

    strcpy(monitor, tmpnam(NULL));
    umask(0002);
    mknod(monitor, S_IFIFO | 0660, 0);

    monitorfd = open(monitor, O_RDONLY | O_NONBLOCK);
    if (monitorfd == -1)
	return;

    syslog(LOGDEBUG, "Opening monitor pipe %s on FD %d", monitor,
	   monitorfd);

    sprintf(line, "monitor %s\n", monitor);
    write(controlfd, line, strlen(line));

    fds->fs->fullcount = 1;
    FD_ZERO(&fds->fs->full);
    FD_SET(monitorfd, &fds->fs->full);
    fds->fs->maxfd = monitorfd;
}

void
monitor_close(void)
{
    if (monitorfd == -1)
	return;

    syslog(LOGDEBUG, "Closing monitor pipe on FD %d", monitorfd);
    close(monitorfd);
    unlink(monitor);
    monitorfd = -1;

    fds->fs->fullcount = 0;
    fds->fs->selcount = 0;
    FD_ZERO(&fds->fs->full);
    FD_ZERO(&fds->fs->sel);
    fds->fs->maxfd = -1;
}

void
fifo_server(void)
{
    port_queue_t   *curr,
                   *prev;
    int             len,
                    i,
                    res;

    if (!(fds->ps->fullcount) || !monitorcount) {
	monitor_close();
	return;
    }

    do {
	len = read(monitorfd, line, 1000);
	if (len <= 0) {
	    continue;
	}

	prev = NULL;
	for (curr = pq, i = 0; curr != NULL && i < monitorcount;) {
	    if (curr->port.flag & FLAG_MONITOR) {
		res = write(curr->port.fd, line, len);
		if (res == -1 && errno == EPIPE) {
		    close_port(&curr->port);
		    curr = prev;
		}
		i++;
	    }
	    curr = (curr ? curr->next : pq);
	}
    } while (len > 0 && monitorcount);
}
