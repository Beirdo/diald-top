/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * (c) 1995 Gavin J. Hurlbut
 * gjhurlbu@beirdo.uplink.on.ca
 *
 */

#ifndef _externs_h
#define _externs_h

/* Included Header Files */
#include <stdio.h>

#ifndef CURSHEAD
  #include <ncurses.h>
#else
  #include CURSHEAD
#endif

#ifdef USE_OBSTACKS
#include <obstack.h>
extern struct obstack the_obstack;
#define lex_free(s) ((char*)(s) == yynullstr ? (void)0 : \
                     (obstack_free(&the_obstack, (char*)(s))))
#else  /* !USE_OBSTACKS */
#include <stdlib.h>
#define lex_free(s) ((char*)(s) == yynullstr ? (void)0 : \
                     (free((char*)(s))))
#endif /* USE_OBSTACKS */

#include "defines.h"
#include "hostrec.h"


/* Global Variables */
extern char *control;
extern char *monitor;
extern char *remoteip;
extern char *format;
extern char *curr_state;
extern char *revision;

extern int controlfd;
extern int monitorfd;
extern FILE *fp_monitor;
extern FILE *input_file;

extern int count;
extern int ip_width;
extern int numbuff;
extern int numeric;
extern int queue;
extern int quit;
extern int quit_if_parse_error;
extern int remotemode;
extern int state;
extern int trunc_local_hosts;

extern int yacc_key;
extern int yydebug;
extern char *yynullstr;
extern void (*yy_input_func)(char*,int*,int);

extern WINDOW *full;
extern WINDOW *sub;
extern char *scrbuff[MAXBUFS];

extern struct hostrec *hosthash[HASHSIZE];


#endif
