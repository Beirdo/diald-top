/*
 * $Id$
 */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <limits.h>
#include <syslog.h>

#include "fds.h"
#include "ports.h"
#include "protos.h"
#include "loglevels.h"

static char rcsid[] = "$Id$";


port_queue_t *pq = NULL;

int port_enqueue( port_t *p )
{
	port_queue_t *np;

	np = (port_queue_t *)malloc(sizeof(port_queue_t));
	if( np == NULL ) {
		return( 0 );
	}

	np->next = pq;
	memcpy( &(np->port), p, sizeof(port_t) );
	pq = np;

	fds->ps->fullcount++;
	FD_SET( np->port.fd, &fds->ps->full );
	fds->ps->maxfd = ( np->port.fd > fds->ps->maxfd
			    ? np->port.fd
			    : fds->ps->maxfd );
	syslog( LOGDEBUG, "FD %d added to PortServer fdset, new max %d",
			   np->port.fd, fds->ps->maxfd );
	return( 1 );
}



void close_port( port_t *p )
{
	port_queue_t *curr, *prev;
	int found = 0;

	if( FD_ISSET( p->fd, &fds->ps->full ) ) {
		syslog( LOGDEBUG, "Port closed at FD %d", p->fd );
		if( p->flag & FLAG_MONITOR ) {
			monitorcount--;
		}
		close( p->fd ); 

		/* All ports closed! */
		if( !pq )	return;

		/* Now dequeue it */
		curr = pq;
		prev = NULL;
		for( ; curr != NULL && !found; prev = curr, curr = curr->next )
		{
			if( memcmp( &(curr->port), p, sizeof(port_t) ) == 0 ) {
				found = 1;
				if( !prev ) {
					/* First in the queue */
					pq = curr->next;
				} else {
					prev->next = curr->next;
				}
	
				fds->ps->fullcount--;
				FD_CLR( p->fd, &fds->ps->full );
				fds->ps->maxfd = adjust_maxfd( fds->ps, p->fd );
				syslog( LOGDEBUG, "FD %d removed from "
						  "PortServer fdset, new "
						  "max %d", p->fd,
					  	  fds->ps->maxfd );
				if( p->userid )	free( p->userid );
				free( curr );
			}
		}
	}
}


thread_fds_t * port_server_init( void )
{
	thread_fds_t *fds;

	fds = (thread_fds_t *)malloc(sizeof(thread_fds_t));

	fds->fullcount = 0;
	fds->selcount = 0;
	FD_ZERO( &fds->full );
	FD_ZERO( &fds->sel );
	fds->maxfd = -1;

	return( fds );
}

void port_server( void )
{
	port_queue_t *curr, *prev;
	int currfd, i, len, count;
	fd_set *rdfds;
	char line[1002];

	count = fds->ps->selcount;
	rdfds = &fds->ps->sel;

	prev = NULL;
	for( curr = pq, i = 0; curr != NULL && i < count; ) {
		currfd = curr->port.fd;
		if( FD_ISSET( currfd, rdfds ) ) {
			i++;
	
			memset( &(line[0]), '\0', 1001 );
			len = read( currfd, line, 1000 );
			if( len > 0 ) {
				if( parse_incoming( line, len, curr ) ) {
					/* The port was closed */
					curr = prev;
				}
			} else {
 				close_port( &curr->port );
				curr = prev;
			}
		}

		prev = curr;
		curr = ( curr ? curr->next : pq );
	}
}

int parse_incoming( char *line, int len, port_queue_t *curr )
{
	char outline[1024];

	if( !strncasecmp( line, "monitor off\r", 12 ) ||
	    !strncasecmp( line, "monitor\r", 8 ) ) {
		if( curr->port.flag & FLAG_MONITOR ) {
			curr->port.flag &= ~FLAG_MONITOR;
			monitorcount--;
		}
	} else 
	if( !strncasecmp( line, "monitor ", 8 ) ) {
		if( !(curr->port.flag & FLAG_MONITOR) ) {
			curr->port.flag |= FLAG_MONITOR;
			monitorcount++;

			if( fds->fs->fullcount == 0 ) {
				monitor_open();
			}
		}
	} else 
	if( !strncasecmp( line, "close", 5 ) ) {
		/* close the connection */
		syslog( LOGDEBUG, "Remote requests close" );
		close_port( &curr->port );
		return( 1 );
	} else
	if( curr->port.flag & FLAG_WRITE ) {
		write( controlfd, line, len );
	} else {
		sprintf( outline, "MESSAGE\nYou have no control access: "
			"Command \"%s\" ignored\n", clean_userid( line ) );
		write( curr->port.fd, outline, strlen( outline ) );
	}
	return( 0 );
}

