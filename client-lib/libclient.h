/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * (c) 1995-2002 Gavin J. Hurlbut
 * gjhurlbu@beirdo.ca
 *
 */

#ifndef _libclient_h
#define _libclient_h


/* Type definitions */

#ifdef HAVE_PTHREAD
typedef pthread_t ThreadID_t;
#else
#ifdef HAVE_FORK
typedef pid_t ThreadID_t;
#else
typedef int ThreadID_t;
#endif
#endif


/* Prototypes */

int ThreadCreate( ThreadID_t *pThreadID, void *(*startFunc)(void *), 
                  void *arg );
int ThreadJoin( ThreadID_t threadID );
void ThreadExit( void *retVal );
char *SharedMemoryGet( int size );
void SharedMemoryRemove( char *buffer, int size );

#endif
