/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * Thread Interface
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
#include "config.h"

#ifdef HAVE_PTHREAD
#include <pthread.h>
#else
#ifdef HAVE_FORK
#include <sys/types.h>
#include <unistd.h>
#endif
#endif

#include "libclient.h"

/*
 * Global Variables 
 */

int ThreadCreate( ThreadID_t *pThreadID, void *(*startFunc)(void *), 
                  void *arg )
{
int result = -1;

#ifdef HAVE_PTHREAD
    result = pthread_create( (pthread_t *)pThreadID, NULL, startFunc, arg );
#else
#ifdef HAVE_FORK
    *pThreadID = fork();
    if( *pThreadID != 0 )
    {
        /* In parent */
        result = 0;
    }
    else
    {
        /* In child */
        startFunc(arg);
        exit( 0 );
    }
#endif
#endif

    return( result );
}

int ThreadJoin( ThreadID_t threadID )
{
    int result = -1;

#ifdef HAVE_PTHREAD
    result = pthread_join( threadID, NULL );
#else
#ifdef HAVE_FORK
    result = (int)waitpid( threadID, NULL, 0 );
    if( result == (int)threadID )
    {
        result = 0;
    }
#endif
#endif

    return( result );
}


void ThreadExit( void *retVal )
{
#ifdef HAVE_PTHREAD
    pthread_exit( retVal );
#else
#ifdef HAVE_FORK
    exit( (unsigned char)((int)retVal) );
#endif
#endif
}

