/*
 * $Id$
 */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "fds.h"
#include "protos.h"


static char rcsid[] = "$Id$";


thread_fds_t muxedfds;


void do_select( void )
{
	mux_fds();
	muxedfds.selcount = select( muxedfds.maxfd+1, &muxedfds.full, NULL,
					NULL, NULL );
	demux_fds();
}


int adjust_maxfd( thread_fds_t *fds, int currfd )
{
	int max, i;

	if( fds->maxfd != currfd )	return( fds->maxfd );

	max = -1;
	for( i = 0; i <= fds->maxfd; i++ ) {
		if( FD_ISSET( i, &fds->full ) ) {
			max = i;
		}
	}

	return( max );
}


void mux_fds( void ) 
{
	int i, max;

	muxedfds.maxfd = -1;
	muxedfds.fullcount = 0;
	FD_ZERO( &muxedfds.full );

	for( i = 0, max = fds->pd->maxfd; i <= max; i++ ) {
		if( FD_ISSET( i, &fds->pd->full ) ) {
			FD_SET( i, &muxedfds.full );
			muxedfds.fullcount++;
		}
	}
	muxedfds.maxfd = ( fds->pd->maxfd > muxedfds.maxfd
				? fds->pd->maxfd
				: muxedfds.maxfd );


	for( i = 0, max = fds->ps->maxfd; i <= max; i++ ) {
		if( FD_ISSET( i, &fds->ps->full ) ) {
			FD_SET( i, &muxedfds.full );
			muxedfds.fullcount++;
		}
	}
	muxedfds.maxfd = ( fds->ps->maxfd > muxedfds.maxfd
				? fds->ps->maxfd
				: muxedfds.maxfd );


	for( i = 0, max = fds->fs->maxfd; i <= max; i++ ) {
		if( FD_ISSET( i, &fds->fs->full ) ) {
			FD_SET( i, &muxedfds.full );
			muxedfds.fullcount++;
		}
	}
	muxedfds.maxfd = ( fds->fs->maxfd > muxedfds.maxfd
				? fds->fs->maxfd
				: muxedfds.maxfd );
}


void demux_fds( void )
{
	int i, max, count;

	FD_ZERO( &fds->pd->sel );
	FD_ZERO( &fds->ps->sel );
	FD_ZERO( &fds->fs->sel );

	fds->pd->selcount = 0;
	fds->ps->selcount = 0;
	fds->fs->selcount = 0;

	for( i = 0; i <= muxedfds.maxfd && count < muxedfds.selcount ; i++ ) {
		if( FD_ISSET( i, &muxedfds.full ) ) {
			if( FD_ISSET( i, &fds->pd->full ) ) {
				FD_SET( i, &fds->pd->sel );
				fds->pd->selcount++;
			}

			if( FD_ISSET( i, &fds->ps->full ) ) {
				FD_SET( i, &fds->ps->sel );
				fds->ps->selcount++;
			}

			if( FD_ISSET( i, &fds->fs->full ) ) {
				FD_SET( i, &fds->fs->sel );
				fds->fs->selcount++;
			}

			count++;
		}
	}
}
