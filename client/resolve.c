/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * (c) 1995 Gavin J. Hurlbut
 * gjhurlbu@beirdo.uplink.on.ca
 *
 * $Log$
 * Revision 2.0  1997/09/28 21:21:15  gjhurlbu
 * Release 2.0
 *
 * Revision 1.6  1997/09/25 06:45:23  gjhurlbu
 * Tidied the client code some
 * Converted some static buffers to dynamically created ones
 *
 * Revision 1.5  1997/09/21 06:00:57  gjhurlbu
 * Fixed the segfault stupidity
 *
 * Revision 1.4  1997/08/30 19:54:16  gjhurlbu
 * Incorporated many patches
 * Fixed redraw
 *
 * Revision 1.3  1997/03/29 17:34:38  gjhurlbu
 * Added RCS IDs to all C files
 * Removed some excess comments
 *
 * Revision 1.2  1996/07/27 21:09:27  gjhurlbu
 * *** empty log message ***
 *
 * Revision 1.1  1996/07/27 20:07:24  gjhurlbu
 * Splitting into separate modules
 *
 */

static char rcsid[] = "$Id$";

/* Included Header Files */
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <malloc.h>

#include "hostrec.h"
#include "defines.h"
#include "prototypes.h"
#include "externs.h"


struct hostrec *hosthash[HASHSIZE];

const char *resolve( const char *aaddr )
{
	struct in_addr addr;
	struct hostrec *hr = NULL;

	if( !inet_aton( aaddr, &addr ) || numeric)
		return aaddr;

	if( probetab(addr) )	hr = gettab(addr);

	if( !hr && !strcmp(curr_state,"UP") ) {
		struct hostent *host;

		host = gethostbyaddr( (char*)&addr, sizeof(addr), AF_INET );
		if( !host ) {
			if( h_errno == HOST_NOT_FOUND ||
			    h_errno == NO_ADDRESS ) {
				hr = gettab(addr);
				hr->name  = strdup(aaddr);
				hr->trunc = strdup(aaddr);
				hr->addr  = addr;
			}
			return aaddr;
		}

		hr = gettab(addr);
		hr->name  = strdup(host->h_name);
		hr->trunc = strdup(host->h_name);
		hr->addr  = addr;
		truncate_hostname(hr->trunc);
	}

	if( !hr )		return aaddr;
	if( trunc_local_hosts )	return( hr->trunc );
	else			return( hr->name );
}


int hash( struct in_addr addr )
{
	unsigned char *c = (unsigned char*)&addr;

	return( (c[0]^c[2]) + ( (c[1]^c[3]) << 8 ) );
}


struct hostrec *gettab( struct in_addr addr )
{
	int h = hash(addr);
	struct hostrec *hr = hosthash[h];

	while( hr ) {
		if( !memcmp( &addr, &(hr->addr), sizeof(struct in_addr) ) )
			return( hr );

		hr=hr->next;
	}

	hr = (struct hostrec *)malloc(sizeof(struct hostrec));
	hr->next = hosthash[h];
	hosthash[h] = hr;
	return( hr );
}
		

int probetab( struct in_addr addr )
{
	struct hostrec *hr = hosthash[hash(addr)];

	while( hr ) {
		if( !memcmp( &addr, &(hr->addr), sizeof(struct in_addr) ) )
			return( 1 );

		hr = hr->next;
	}
	return( 0 );
}


void truncate_hostname( char *hostname )
{
	static char mydomain[256];
	char *ptr;      

	if( mydomain[0] == '\0' ) {
		getdomainname( mydomain, 256 );
	}

	if( (ptr = strstr( hostname, mydomain )) && (ptr > hostname) ) {
		ptr[-1] = '\0';
	}
}


const char *service( int port, const char *proto )
{
	char *sstr;
        struct servent *serv;
        static int first_time = 1;

	if (first_time) {
		first_time = 0;
		setservent( 1 );
        }
	
	serv = getservbyport( htons(port), proto );

#ifdef USE_OBSTACKS

	if( !serv ) {
		sstr = obstack_alloc( &the_obstack, 6 );
		snprintf( sstr, 6, "%-5d", port );
	} else {
		/* do it by hand to save a scan by strlen */
		register char * p = serv->s_name;

		while( *p ) {
			obstack_1grow( &the_obstack, *p );
			p++;
		}

		obstack_1grow( &the_obstack, 0 );
		sstr = obstack_finish( &the_obstack );
        }

#else  /* !USE_OBSTACKS */

	if( !serv ) {
		sstr = (char *)malloc(6);
		if( sstr ) {
			snprintf( sstr, 6, "%-5d", port );
		}
	} else {
		sstr = strdup( serv->s_name );
	}

#endif /* USE_OBSTACKS */

	return( sstr ? sstr : yynullstr );
}
	
