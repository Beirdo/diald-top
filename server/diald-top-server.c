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
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>

#include "fds.h"
#include "ports.h"
#include "protos.h"
#include "loglevels.h"

static char     rcsid[] =
    "$Id$";

fds_t          *fds;

int
main(void)
{
    fds = (fds_t *) alloca(sizeof(fds_t));

    openlog("diald-top-server", LOGOPTIONS, LOGFACILITY);

    detach_process();

    signal(SIGINT, sigint);
    signal(SIGTERM, sigint);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    syslog(LOGNOTICE, "Starting diald-top-server");

    fds->pd = port_daemon_init();
    fds->ps = port_server_init();
    fds->fs = fifo_server_init();

    read_conf();

    signal(SIGHUP, reread_conf);

    while (1) {
	do_select();
	if (fds->pd->selcount)
	    port_daemon();
	if (fds->ps->selcount)
	    port_server();
	if (fds->fs->selcount)
	    fifo_server();
    }

    openlog("diald-top-server", LOGOPTIONS | LOG_PERROR, LOGFACILITY);
    syslog(LOGNOTICE, "Aborting");
    closelog();
    _exit(2);
}

void
sigint(int signum)
{
    signal(signum, sigint);
    syslog(LOGNOTICE, "Caught signal");
    _exit(2);
}

void
detach_process(void)
{
    pid_t           pid;

    if ((pid = fork()) > 0) {
	syslog(LOGDEBUG, "Detached:  PID %d", pid);
	exit(0);		/*
				 * Quit the original to detach 
				 */
    } else if (pid == -1) {
	openlog("diald-top-server", LOGOPTIONS | LOG_PERROR, LOGFACILITY);
	syslog(LOGERR, "fork: %m");
	syslog(LOGERR, "couldn't fork new process");
	exit(1);
    }

    if ((pid = setsid()) == -1) {
	openlog("diald-top-server", LOGOPTIONS | LOG_PERROR, LOGFACILITY);
	syslog(LOGERR, "setsid: %m");
	syslog(LOGERR, "setsid failed");
	exit(1);
    }
}
