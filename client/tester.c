#include <stdio.h>
#include "y.tab.h"

static char rcsid[] = "$Id$";

#ifdef LEXONLY

extern int yylex( void );
extern char *yytext;
extern int yyleng;

YYSTYPE yylval;

int main( void )
{
	int ret, i;

	for( ret = yylex(); ret != _EOF; ret = yylex() ) {
		switch( ret ) {
		case _STATE:
			printf( "_STATE " );
			break;

		case _INTERFACE:
			printf( "_INTERFACE " );
			break;

		case _STATUS:
			printf( "_STATUS " );
			break;

		case _LOAD:
			printf( "_LOAD " );
			break;

		case _MESSAGE:
			printf( "_MESSAGE " );
			break;

		case _QUEUE:
			printf( "_QUEUE " );
			break;

		case _ENDQUEUE:
			printf( "_ENDQUEUE " );
			break;

		case _TIME:
			printf( "_TIME " );
			break;

		case _NUM:
			printf( "_NUM " );
			break;

		case _IP:
			printf( "_IP " );
			break;

		case _PORT:
			printf( "_PORT " );
			yytext++;
			yyleng--;
			break;

		case _NEWLINE:
			printf( "\n" );
			break;

		case _CHAR:
			printf( "_CHAR " );
			break;

		}
		if( ret != _NEWLINE ) {
			printf( "(" );
			for( i = 0; i < yyleng; i++ ) {
				printf( "%c", yytext[i] );
			}
			printf( ") " );
		}
	}

	return( 0 );
}

#else

extern FILE *yyin;
extern int yyparse( void );
extern int yacc_eof;

#ifdef YYDEBUG
extern int yydebug;
#endif

int main( int argc, char **argv )
{
	int ret;

#ifdef YYDEBUG
	if( argc >= 2 ) {
		yydebug = atoi(argv[1]);
	} else {
		yydebug = 0;
	}
#endif

	if( argc >= 3 ) {
		yyin = fopen( argv[2], "rt" );
		if( !yyin )	yyin = stdin;
	} else {
		yyin = stdin;
	}

	yacc_eof = 0;

	while( !yacc_eof ) {
		ret = yyparse();

		printf( "Parse Result = %d\n\n", ret );
	}

	return( ret );
}

#endif
