/*
 * $Id$
 */

#ifndef __protos_h
#define __protos_h


#include <unistd.h>
#include <stdio.h>
#include "fds.h"
#include "ports.h"


/* diald-top-server.c */
int main( void );
void sigint( int signum );
void detach_process( void );


/* fifo-server.c */
thread_fds_t *fifo_server_init( void );
void monitor_open( void );
void monitor_close( void );
void fifo_server( void );


/* port-daemon.c */
thread_fds_t * port_daemon_init( void );
void port_daemon( void );
char * reap_userid( char *line );
__inline char * clean_userid( char *line );


/* port-queue.c */
int port_enqueue( port_t *p );
thread_fds_t * port_server_init( void );
void port_server( void );
int parse_incoming( char *line, int len, port_queue_t *curr );
void close_port( port_t *p );


/* selection.c */
void do_select( void );
int adjust_maxfd( thread_fds_t *fds, int currfd );
void mux_fds( void );
void demux_fds( void );

/* authenticate.c */
void auth_add( char *userid, unsigned long int ip, unsigned long int netmask );
int auth_check( char *userid, unsigned long int ip );
void read_conf( void );
void reread_conf( int signum );
void reauthenticate_users( void );


/* Externals */
extern int controlfd;
extern int monitorfd;
extern int monitorcount;
extern FILE *yyin;
extern int yyparse( void );
extern fds_t *fds;

#endif

