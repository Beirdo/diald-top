/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * (c) 1995 Gavin J. Hurlbut
 * gjhurlbu@beirdo.ott.uplink.on.ca
 *
 */

#ifndef _hostrec_h
#define _hostrec_h

/*
 * Included Header Files 
 */
#include <netinet/in.h>
#include <sys/types.h>
#include <time.h>

struct hostrec {
    struct in_addr  addr;
    char           *name;
    char           *trunc;
    struct hostrec *next;
    time_t          time_resolved;
};

#endif
