%{
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "protos.h"

extern int yylex( void );

#define YYERROR_VERBOSE 

#define lex_free(x) \
	if(x != 0) free( (void *)(x) );

#define YYSTYPE unsigned long int

%}

%token _USERID
%token _IP
%token _NEWLINE

%start newline

%%

ip:	_IP
	{
		struct in_addr addr;

		inet_aton( (char *)$1, &addr );
		lex_free( $1 );

		$$ = addr.s_addr;
	}
	;


newline: _USERID ip ip _NEWLINE
	{
		auth_add( (char *)$1, (unsigned long int)$2,
				(unsigned long int)$3 );
		lex_free( $1 );
	}
	| newline _USERID ip ip _NEWLINE
	{
		auth_add( (char *)$2, (unsigned long int)$3,
				(unsigned long int)$4 );
		lex_free( $2 );
	}
	| newline _NEWLINE
	| _NEWLINE
	;

%%

static char rcsid[] = "$Id$";

int yyerror( char *string )
{
	fprintf( stderr, "%s\n", string );
        fprintf( stderr, "lookahead symbol = %d\n", yychar);
        exit(1);
}
