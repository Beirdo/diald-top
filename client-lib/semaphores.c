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
#ifdef HAVE_SEMGET
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#endif
#endif

#include "libclient.h"

/*
 * Global Variables 
 */

int SemaphoreCreate( SemID_t *pSemID, int initVal )
{
    int result = -1;

#ifdef HAVE_PTHREAD
    pthread_mutex_init( &pSemID->mutex, NULL );
    pSemID->value = initVal;
    pthread_cond_init( &pSemID->condAboveThresh, NULL );
    result = 0;
#else
#ifdef HAVE_SEMGET
    *pSemID = semget( IPC_PRIVATE, 1, 00660 );
    semctl( *pSemID, 0, SETVAL, initVal );
    result = 0;
#endif
#endif

    return( result );
}


void SemaphoreDestroy( SemID_t *pSemID )
{
#ifdef HAVE_PTHREAD
    pthread_mutex_destroy( &pSemID->mutex );
    pthread_cond_destroy( &pSemID->condAboveThresh );
#else
#ifdef HAVE_SEMGET
    semctl( *pSemID, 0, IPC_RMID, 0 );
#endif
#endif
}

void SemaphoreTake( SemID_t *pSemID )
{
#ifdef HAVE_PTHREAD
    pthread_mutex_lock( &pSemID->mutex );
    if( pSemID->value == 0 )
    {
        pthread_cond_wait( &pSemID->condAboveThresh, &pSemID->mutex );
    }
    pSemID->value--;
    pthread_mutex_unlock( &pSemID->mutex );
#else
#ifdef HAVE_SEMGET
    struct sembuf semaphoreBuffer;
    semaphoreBuffer.sem_num = 0;
    semaphoreBuffer.sem_op  = -1;
    semaphoreBuffer.sem_flg = 0;
    semop( *pSemID, &semaphoreBuffer, 1 );
#endif
#endif
}



void SemaphoreGive( SemID_t *pSemID )
{
#ifdef HAVE_PTHREAD
    pthread_mutex_lock( &pSemID->mutex );
    pSemID->value++;
    pthread_cond_signal( &pSemID->condAboveThresh );
    pthread_mutex_unlock( &pSemID->mutex );
#else
#ifdef HAVE_SEMGET
    struct sembuf semaphoreBuffer;
    semaphoreBuffer.sem_num = 0;
    semaphoreBuffer.sem_op  = 1;
    semaphoreBuffer.sem_flg = 0;
    semop( *pSemID, &semaphoreBuffer, 1 );
#endif
#endif
}


