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

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "config.h"
#include "protos.h"
#include "loglevels.h"
#include "ports.h"

static char     rcsid[] =
    "$Id$";

typedef struct {
    char           *userid;
    unsigned long int ip;
    unsigned long int netmask;
} auth_data_t;

typedef struct auth_queue {
    struct auth_queue *next;
    auth_data_t     conf;
} auth_queue_t;

auth_queue_t   *aq = NULL;

void
auth_add(char *userid, unsigned long int ip, unsigned long int netmask)
{
    auth_queue_t   *newauth;
    char            line[256] = "";

    newauth = (auth_queue_t *) malloc(sizeof(auth_queue_t));
    newauth->next = aq;
    newauth->conf.userid = strdup(userid);
    newauth->conf.ip = ip & netmask;
    newauth->conf.netmask = netmask;

    sprintf(line, "Added user: %s@%s/", userid, inet_ntoa(ipaddress(ip)));
    strcat(line, inet_ntoa(ipaddress(netmask)));
    syslog(LOGDEBUG, "%s", line);

    aq = newauth;
}

int
auth_check(char *userid, unsigned long int ip)
{
    auth_queue_t   *curr;
    unsigned long int ipcheck;
    char            line[256] = "";

    sprintf(line, "Checking user: %s@%s -- ", userid,
	    inet_ntoa(ipaddress(ip)));

    for (curr = aq; curr != NULL; curr = curr->next) {
	if (!strcasecmp(curr->conf.userid, userid)) {
	    /*
	     * Name matches, check the IP 
	     */
	    ipcheck = ip & curr->conf.netmask;
	    if (ipcheck == curr->conf.ip) {
		/*
		 * we have a weiner 
		 */
		strcat(line, "full access");
		syslog(LOGDEBUG, "%s", line);
		return (1);
	    }
	}
    }

    /*
     * Not matched -- read-only access 
     */
    strcat(line, "read-only access");
    syslog(LOGDEBUG, "%s", line);
    return (0);
}

void
read_conf(void)
{
    yyin = fopen(CONF_FILE, "rt");
    if (!yyin)
	return;

    yyparse();
    fclose(yyin);
}

void
reread_conf(int signum)
{
    auth_queue_t   *curr,
                   *next;

    signal(SIGHUP, SIG_IGN);

    /*
     * remove old conf 
     */
    for (curr = aq; curr != NULL; curr = next) {
	next = curr->next;
	free(curr->conf.userid);
	free(curr);
    }
    aq = NULL;

    /*
     * read it again 
     */
    syslog(LOGNOTICE, "Configuration re-read (signal %d)", signum);
    read_conf();
    reauthenticate_users();

    signal(SIGHUP, reread_conf);
}

void
reauthenticate_users(void)
{
    port_queue_t   *curr;
    port_t         *p;
    char           *full =
	"MESSAGE\nServer configuration reloaded - full access "
	"granted\n";
    char           *ro =
	"MESSAGE\nServer configuration reloaded - read-only "
	"access granted\n";
    int             f_len = strlen(full);
    int             r_len = strlen(ro);

    curr = pq;
    for (; curr != NULL; curr = curr->next) {
	p = &curr->port;
	p->flag &= ~(FLAG_READ | FLAG_WRITE);
	if (auth_check(p->userid, p->sa.sin_addr.s_addr)) {
	    write(p->fd, full, f_len);
	    p->flag |= (FLAG_READ | FLAG_WRITE);
	} else {
	    write(p->fd, ro, r_len);
	    p->flag |= FLAG_READ;
	}
    }
}
