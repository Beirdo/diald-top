/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * Shared Memory Interface
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

#include <stdlib.h>
#ifdef HAVE_PTHREAD
#include <pthread.h>
#else
#ifdef HAVE_LIBRT
/* use shm_open/mmap, shm_unlink */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#else
/* use shmget/shmat, shmdt/shmctl(IPC_RMID) */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#endif


/*
 * Global Variables 
 */

void SharedMemoryDestroy( int exitLevel, void *arg );

char *SharedMemoryGet( int size )
{
    char *buffer = NULL;
    int fd = -1;

#ifdef HAVE_PTHREAD
    buffer = (char *)malloc(size);
#else
#ifdef HAVE_LIBRT
/* shm_open, mmap */
    fd = shm_open( "/diald-top", O_RDWR | O_CREAT, 00660 );
    ftruncate( fd, size );
    buffer = (char *)mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, 0,
                           fd );
    close( fd );
#else
/* shmget */
    fd = shmget( IPC_PRIVATE, size, IPC_CREAT | 00660 );
    buffer = (char *)shmat( fd, NULL, 0 );
    on_exit( SharedMemoryDestroy, (void *)fd );
#endif
#endif
    return( buffer );
}



void SharedMemoryRemove( char *buffer, int size )
{
#ifdef HAVE_PTHREAD
    (void)size;
    free( buffer );
#else
#ifdef HAVE_LIBRT
    /* shm_unlink */
    munmap( buffer, size );
    shm_unlink( "/diald-top" );
#else
    /* shmdt */
    (void)size;
    shmdt( buffer );
#endif
#endif
}


void SharedMemoryDestroy( int exitLevel, void *arg )
{
#ifdef HAVE_PTHREAD
    /* Unused */
#else
#ifdef HAVE_LIBRT
    /* Unused */
#else
    /* Need to destroy the shared memory segment received with shmget() */
    shmctl( (int)arg, IPC_RMID, NULL );
#endif
#endif
}
