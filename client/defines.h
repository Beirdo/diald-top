/*
 * $Id$
 *
 * DialD Packet Statistics Program
 * (c) 1995 Gavin J. Hurlbut
 * gjhurlbu@beirdo.uplink.on.ca
 *
 */

#ifndef _defines_h
#define _defines_h

/* Included Header Files */

/* Global Definitions */
#define HEAD_LINES	9
#define HASHSIZE	65536
#define LINELENGTH	256
#define MAXBUFS		256
#define MAX_QUERY_LOAD  200
#define HOSTREC_EXPIRE  (60*60*24*2) /* 2 days */

#define LINE_COPYRIGHT	0
#define LINE_REVISION 	1
#define LINE_STATE	3
#define LINE_INTERFACE	4
#define LINE_MESSAGE	5
#define LINE_COMMAND	6
#define LINE_HEADERS	7
#define LINE_UNDERLINE	8


#endif

