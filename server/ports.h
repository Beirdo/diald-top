/*
 * $Id$
 */

#ifndef __ports_h
#define __ports_h

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define FLAG_READ	0x00000001
#define FLAG_WRITE	0x00000002
#define FLAG_MONITOR	0x00000004

typedef struct {
    int             fd;
    struct sockaddr_in sa;
    unsigned int    flag;
    char           *userid;
} port_t;

typedef struct port_queue {
    struct port_queue *next;
    port_t          port;
} port_queue_t;

extern port_queue_t *pq;

#define sockaddr(x)	((struct sockaddr *)(x))
#define socklen		sizeof(struct sockaddr)
#define ipaddress(x)	(*((struct in_addr *)&(x)))
#endif
