%{
#include <stdio.h>
#include "parser.h"
#include "externs.h"

int             yacc_key;
int             quit_if_parse_error;

extern int      yylex(void);
extern int      lex_testkey(void);

#define YYERROR_VERBOSE
%}

%token _STATE
%token _INTERFACE
%token _STATUS
%token _LOAD
%token _MESSAGE
%token _QUEUE
%token _ENDQUEUE
%token _TIME
%token _NUM
%token _IP
%token _NEWLINE
%token _PORT
%token _STRING
%token _WOULDBLOCK

%start blurbs

%%

num:	_NUM _NEWLINE
	;

time:	_TIME _NEWLINE
	| _WOULDBLOCK
	{
		YYACCEPT;
	}
	;

string:	_STRING
	| string _STRING
	;

stringnl:	string _NEWLINE
	;

ip:	_IP
	;

ipnl:	_IP _NEWLINE
	;

port:	_PORT
	;

blurbs:                        /* empty */
        | blurbs blurb
        | error blurbs
	| belch
	;

belch:	blurbs time
	{
		lex_free( $2 );
	}
	| blurbs string
	{
		lex_free( $2 );
	}
	| blurbs _ENDQUEUE
	| blurbs _IP
	{
		lex_free( $2 );
	}
	| blurbs _PORT
	| blurbs _NEWLINE
	| blurbs _NUM
        ;

blurb:	status num num num num num num time time time
	{
		do_status( $2, $3, $4, $5, $6, $7, (char *)$8, (char *)$9, 
			(char *)$10 );
		lex_free( $10 );
		lex_free( $9 );
		lex_free( $8 );
		if( lex_testkey() )	YYACCEPT;
	}
	| load num num
	{
		do_load( $2, $3 );
		lex_testkey();          YYACCEPT;
	}
	| interface stringnl ipnl ipnl
	{
		do_interface( (char *)$2, (char *)$3, (char *)$4 );
                lex_free( $4 );
		lex_free( $3 );
		lex_free( $2 );
		if( lex_testkey() )	YYACCEPT;
	}
	| state stringnl
	{
		do_state( (char *)$2 );
                lex_free( $2 );
		if( lex_testkey() )	YYACCEPT;
	}
	| message stringnl
	{
		do_message( (char *)$2 );
                lex_free( $2 );
		if( lex_testkey() )	YYACCEPT;
	}
	| queue packets endqueue
	{
		if( lex_testkey() )	YYACCEPT;
	}
        | queue error endqueue
	{
		if( lex_testkey() )	YYACCEPT;
	}
	;

status:	_STATUS _NEWLINE
	;

load:	_LOAD  _NEWLINE
	;

interface:	_INTERFACE _NEWLINE
	;

state:	_STATE _NEWLINE
        ;

message: _MESSAGE _NEWLINE
	;

queue:	_QUEUE _NEWLINE
	{
		do_queue();
	}
	;

endqueue: _ENDQUEUE _NEWLINE
        {
		do_endqueue();
        }
        ;

packets:                       /* empty */
         | packets packet
         ;

packet: string ip port ip port time
	{
		do_packet( (char *)$1, (char *)$2, $3, (char *)$4, $5, 
			(char *)$6 );
                lex_free( $6 );
		lex_free( $4 );
		lex_free( $2 );
		lex_free( $1 );
	}
	| _NUM ip port ip port time
	{
                char num_string[20];
                sprintf(num_string, "%d", $2);
		do_packet( num_string, (char *)$2, $3, (char *)$4, $5, 
			(char *)$6 );
		lex_free( $6 );
		lex_free( $4 );
		lex_free( $2 );
	}
	| _WOULDBLOCK
%%

static char     rcsid[] =
    "$Id$";

int
yyerror(char *string)
{
    if (yychar == _WOULDBLOCK)
	return (0);
    fprintf(stderr, "%s\n", string);
    fprintf(stderr, "lookahead symbol = %d\n", yychar);
    if (quit_if_parse_error)
	exit(1);
}
