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

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>

#include "config.h"
#include "fds.h"
#include "ports.h"
#include "protos.h"
#include "loglevels.h"

static char     rcsid[] =
    "$Id$";
static port_t   s_list,
                s_in,
                s_ident;

thread_fds_t   *
port_daemon_init(void)
{
    thread_fds_t   *fds;
    int             res;

    fds = (thread_fds_t *) malloc(sizeof(thread_fds_t));

    fds->selcount = 0;
    FD_ZERO(&fds->sel);
    FD_ZERO(&fds->full);

    s_list.sa.sin_family = AF_INET;
    s_list.sa.sin_addr.s_addr = 0;
    s_list.sa.sin_port = htons(1313);

    s_ident.sa.sin_family = AF_INET;
    s_ident.sa.sin_port = htons(113);

    s_list.fd = socket(AF_INET, SOCK_STREAM, 6);
    if (bind(s_list.fd, sockaddr(&(s_list.sa)), socklen) < 0) {
	openlog("diald-top-server", LOGOPTIONS | LOG_PERROR, LOGFACILITY);
	syslog(LOGERR, "Couldn't bind port 1313, aborting: %m");
	_exit(1);
    }
    res = listen(s_list.fd, 32);
    syslog(LOGDEBUG, "Listening to FD %d", s_list.fd);

    s_list.sa.sin_port = 0;

    fds->fullcount = 1;
    FD_SET(s_list.fd, &fds->full);
    fds->maxfd = s_list.fd;

    return (fds);
}

void
port_daemon(void)
{
    int             connlen,
                    res;
    char           *userid;
    char           *line,
                   *user;
    struct sockaddr_in name;

    line = (char *) alloca(257);
    user = (char *) alloca(257);

    connlen = sizeof(s_in.sa);
    s_in.fd = accept(s_list.fd, sockaddr(&(s_in.sa)), &connlen);
    if (s_in.fd < 0) {
	syslog(LOGERR, "accept: %m");
    }

    s_ident.sa.sin_addr.s_addr = s_in.sa.sin_addr.s_addr;
    syslog(LOGDEBUG, "Incoming connection from %s(%d) - FD %d",
	   inet_ntoa(s_in.sa.sin_addr), connlen, s_in.fd);

    connlen = sizeof(name);
    if (getsockname(s_in.fd, sockaddr(&name), &connlen) < 0) {
	syslog(LOGERR, "getsockname failing: %m");
    } else {
	syslog(LOGDEBUG, "Incoming connection on %s(%d)",
	       inet_ntoa(name.sin_addr), connlen);
    }

    s_ident.fd = socket(AF_INET, SOCK_STREAM, 6);
    name.sin_port = 0;
    if (bind(s_ident.fd, sockaddr(&name), socklen) < 0) {
	syslog(LOGERR, "bind on identd failing: %m");
    }

    syslog(LOGDEBUG, "Attempting identd connect to %s",
	   inet_ntoa(s_in.sa.sin_addr));

    if (connect(s_ident.fd, sockaddr(&(s_ident.sa)), socklen) < 0) {
	syslog(LOGERR, "connect on identd failing: %m");
	write(s_in.fd, "MESSAGE\nUnauthorized Access -- "
	      "Monitor only (no ident)\n", 55);
	s_in.flag = FLAG_READ;
	s_in.userid = NULL;
    } else {
	sprintf(line, "%d, 1313\n", ntohs(s_in.sa.sin_port));
	write(s_ident.fd, line, strlen(line));
	read(s_ident.fd, line, 256);
	close(s_ident.fd);

	syslog(LOGDEBUG, "Ident returned: %s", clean_userid(line));
	userid = reap_userid(line);
	s_in.userid = strdup(userid);

	if (auth_check(userid, s_in.sa.sin_addr.s_addr)) {
	    sprintf(user, "MESSAGE\nUser %s@%s connected "
		    "-- Full access\n", userid,
		    inet_ntoa(s_in.sa.sin_addr));
	    s_in.flag = FLAG_READ | FLAG_WRITE;
	} else {
	    sprintf(user, "MESSAGE\nUser %s@%s connected "
		    "-- Monitor access\n", userid,
		    inet_ntoa(s_in.sa.sin_addr));
	    s_in.flag = FLAG_READ;
	}
	write(s_in.fd, user, strlen(user));
    }

    if (!port_enqueue(&s_in)) {
	if (s_in.userid)
	    free(s_in.userid);
	close(s_in.fd);
    }
}

char           *
reap_userid(char *line)
{
    char           *res;

    for (; *line != ':' && *line; line++);
    if (*line)
	line++;

    for (; *line != ':' && *line; line++);
    if (*line)
	line++;

    for (; *line != ':' && *line; line++);
    if (*line)
	line++;

    res = line;

    for (; *line != '\n' && *line != '\r' && *line; line++);
    *line = '\0';

    res = clean_userid(res);
    return (res);
}

__inline char  *
clean_userid(char *line)
{
    char           *res;

    res = line;

    for (; *line != '\n' && *line != '\r' && *line; line++);
    *line = '\0';

    for (; *res == ' '; res++);

    return (res);
}
