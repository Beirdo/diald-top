/*
 * $Id$
 */

#ifndef __fds_h
#define __fds_h

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    int             fullcount;
    fd_set          full;
    int             maxfd;
    int             selcount;
    fd_set          sel;
} thread_fds_t;

typedef struct {
    thread_fds_t   *pd;
    thread_fds_t   *ps;
    thread_fds_t   *fs;
} fds_t;

#endif
