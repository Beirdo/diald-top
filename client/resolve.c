/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * Perform reverse DNS resolution on IPs
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
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "hostrec.h"
#include "defines.h"
#include "prototypes.h"
#include "externs.h"

struct hostrec *hosthash[HASHSIZE];

const char     *
resolve(const char *aaddr)
{
#ifdef HAVE_GETHOSTBYADDR
    struct in_addr  addr;
    struct hostrec *hr = NULL;

    if (!
#if HAVE_INET_PTON
       inet_pton(AF_INET, aaddr, &addr)
#else
       inet_aton(aaddr, &addr)
#endif
	  || numeric)
	return aaddr;

    if (probetab(addr))
	hr = gettab(addr);

    if ((!hr || now - hr->time_resolved > HOSTREC_EXPIRE) &&
	!strcmp(curr_state, "UP") &&
	rxload[0] + rxload[1] + rxload[2] < MAX_QUERY_LOAD &&
	txload[0] + txload[1] + rxload[2] < MAX_QUERY_LOAD) {
	struct hostent *host;

	host = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);
	if (!host) {
	    if (h_errno == HOST_NOT_FOUND || h_errno == NO_ADDRESS) {
		if (!hr)
		    hr = gettab(addr);
		else {
		    free(hr->name);
		    free(hr->trunc);
		}
		hr->name = strdup(aaddr);
		hr->trunc = strdup(aaddr);
		hr->addr = addr;
		hr->time_resolved = now;
	    }
	    return aaddr;
	}

	if (!hr)
	    hr = gettab(addr);
	else {
	    free(hr->name);
	    free(hr->trunc);
	}
	hr->name = strdup(host->h_name);
	hr->trunc = strdup(host->h_name);
	hr->addr = addr;
	hr->time_resolved = now;
#ifdef HAVE_GETDOMAINNAME
	truncate_hostname(hr->trunc);
#endif /* HAVE_GETDOMAINNAME */
    }

    if (!hr)
	return aaddr;

    if (trunc_local_hosts)
	return (hr->trunc);
    else
	return (hr->name);
#else  /* !HAVE_GETHOSTBYADDR */
    return aaddr;
#endif /* HAVE_GETHOSTBYADDR */

}

#ifdef HAVE_GETHOSTBYADDR
int
hash(struct in_addr addr)
{
    unsigned char  *c = (unsigned char *) &addr;

    return ((c[0] ^ c[2]) + ((c[1] ^ c[3]) << 8));
}

struct hostrec *
gettab(struct in_addr addr)
{
    int             h = hash(addr);
    struct hostrec *hr = hosthash[h];

    while (hr) {
	if (!memcmp(&addr, &(hr->addr), sizeof(struct in_addr)))
	    return (hr);

	hr = hr->next;
    }

    hr = (struct hostrec *) malloc(sizeof(struct hostrec));
    hr->next = hosthash[h];
    hosthash[h] = hr;
    return (hr);
}

int
probetab(struct in_addr addr)
{
    struct hostrec *hr = hosthash[hash(addr)];

    while (hr) {
	if (!memcmp(&addr, &(hr->addr), sizeof(struct in_addr)))
	    return (1);

	hr = hr->next;
    }
    return (0);
}

#ifdef HAVE_GETDOMAINNAME
void
truncate_hostname(char *hostname)
{
    static char     mydomain[256];
    char           *ptr;

    if (mydomain[0] == '\0') {
	getdomainname(mydomain, 256);
    }

    if ((ptr = strstr(hostname, mydomain)) && (ptr > hostname)) {
	ptr[-1] = '\0';
    }
}
#endif /* HAVE_GETDOMAINNAME */
#endif /* HAVE_GETHOSTBYADDR */

const char     *
service(int port, const char *proto)
{
#ifdef HAVE_GETSERVBYPORT
    char           *sstr;
    struct servent *serv;
    static int      first_time = 1;

    if (first_time) {
	first_time = 0;
	setservent(1);
    }

    serv = getservbyport(htons(port), proto);

#ifdef HAVE_OBSTACK

    if (!serv) {
	sstr = obstack_alloc(&the_obstack, 6);
	snprintf(sstr, 6, "%-5d", port);
    } else {
	/*
	 * do it by hand to save a scan by strlen 
	 */
	register char  *p = serv->s_name;

	while (*p) {
	    obstack_1grow(&the_obstack, *p);
	    p++;
	}

	obstack_1grow(&the_obstack, 0);
	sstr = obstack_finish(&the_obstack);
    }

#else				/*
				 * !HAVE_OBSTACK
				 */

    if (!serv) {
	sstr = (char *) malloc(6);
	if (sstr) {
	    snprintf(sstr, 6, "%-5d", port);
	}
    } else {
	sstr = strdup(serv->s_name);
    }

#endif				/*
				 * HAVE_OBSTACK
				 */

    return (sstr ? sstr : yynullstr);
#else /* !HAVE_GETSERVBYPORT */
    char           *sstr;

#ifdef HAVE_OBSTACK
    sstr = obstack_alloc(&the_obstack, 6);
    snprintf(sstr, 6, "%-5d", port);
#else				/*
				 * !HAVE_OBSTACK
				 */
    sstr = (char *) malloc(6);
    if (sstr) {
        snprintf(sstr, 6, "%-5d", port);
    }
#endif				/*
				 * HAVE_OBSTACK
				 */
    return (sstr ? sstr : yynullstr);

#endif /* HAVE_GETSERVBYPORT */
}
