/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * (c) 1995 Gavin J. Hurlbut
 * gjhurlbu@beirdo.ott.uplink.on.ca
 *
 */

#ifndef _prototypes_h
#define _prototypes_h

/*
 * Included Header Files 
 */
#include <netinet/in.h>
#include "hostrec.h"

/*
 * Protoypes 
 */

/*
 * diald-top.c 
 */
int             main(int argc, char **argv);
int             read_command_line(int argc, char **argv);
char           *fix_version(char *in);

/*
 * pipe.c 
 */
void            close_fifo(void);
int             fifo_command(char *command);
void            init_fifo(void);
int             read_fifo(void);
char           *strstrip(char *line);
int             strtcmp(char *str1, char *str2);
void            validate_command(char *command);

/*
 * resolve.c 
 */
#ifdef HAVE_GETHOSTBYADDR
struct hostrec *gettab(struct in_addr addr);
int             hash(struct in_addr addr);
int             probetab(struct in_addr addr);
#ifdef HAVE_GETDOMAINNAME
void            truncate_hostname(char *hostname);
#endif /* HAVE_GETDOMAINNAME */
#endif /* HAVE_GETHOSTBYADDR */
const char     *resolve(const char *aaddr);
const char     *service(int port, const char *proto);

/*
 * screen.c 
 */
void            close_screen(void);
void            display_usage(int argc, char **argv);
void            get_command(char *command);
void            helpscreen(void);
void            show_help_commands(void);
void            print_help(int x, char *line);
void            init_screen(void);
void            parse_keyboard(void);
char           *strrpt(char ch, int count);
void            update_screen(void);

/*
 * monitor.l 
 */
void            yy_log_input(char *buf, int *result, int size);
void            yy_nolog_input(char *buf, int *result, int size);

#endif
