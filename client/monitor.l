%option always-interactive
%{
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/times.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>


#include "y.tab.h"
#include "externs.h"
#include "prototypes.h"

YYSTYPE lex_string( void );
YYSTYPE lex_char( void );
YYSTYPE lex_num( void );

char *yynullstr = "";
int yacc_eof;
int yacc_block;
extern int errno;

FILE *input_file;
void (*yy_input_func)(char*,int*,int) = yy_nolog_input;

#define YY_INPUT(buf,result,max_size) yy_input_func(buf,&result,max_size)

void yy_log_input(char *buf,int *result,int max_size) {
	int c;
	struct tms _tms;
	static clock_t clock_start = 0;
	static FILE *oldyyin = NULL;
	static int yyfd;
	fd_set fd;
	struct timeval tv;

	if (!clock_start) {
		clock_start = times(&_tms);
	}

	if( oldyyin != yyin ) {
		oldyyin = yyin;
		yyfd = fileno( yyin );
	}

	FD_ZERO( &fd );
	FD_SET( yyfd, &fd );

	tv.tv_sec = 0;
	tv.tv_usec = 500000ul;

	if( !select( yyfd+1, &fd, NULL, NULL, &tv ) ) {
		yacc_block = 1;
		*result = 0;
		return;
	}

	c = read( yyfd, buf, max_size );
	yacc_block = 0;
	fprintf(input_file,"%ld> %s",times(&_tms)-clock_start,buf);
	*result = ( c > 0 ? c : YY_NULL );
}

void yy_nolog_input(char *buf,int *result,int max_size) {
	int c;
	static FILE *oldyyin = NULL;
	static int yyfd;
	fd_set fd;
	struct timeval tv;

	if( oldyyin != yyin ) {
		oldyyin = yyin;
		yyfd = fileno( yyin );
	}

	FD_ZERO( &fd );
	FD_SET( yyfd, &fd );

	tv.tv_sec = 0;
	tv.tv_usec = 500000ul;

	if( !select( yyfd+1, &fd, NULL, NULL, &tv ) ) {
		yacc_block = 1;
		*result = 0;
		return;
	}

	c = read( yyfd, buf, max_size );
	yacc_block = 0;
	*result = ( c > 0 ? c : YY_NULL );
}

%}

DIGIT	[0-9]
NONDIGIT [^0-9 \n]
NOSLASH [^/ \n]
TIME	[0-9][0-9]:[0-9][0-9]:[0-9][0-9]
IP	[0-9]{1,3}"."[0-9]{1,3}"."[0-9]{1,3}"."[0-9]{1,3}
PORT	"/"[1-9][0-9]*

%x in_message
%s pre_message

%%

^STATE		return( _STATE );
^INTERFACE	return( _INTERFACE );
^STATUS		return( _STATUS );
^LOAD		return( _LOAD );
^QUEUE		return( _QUEUE );
^END" "QUEUE	return( _ENDQUEUE );
^MESSAGE	BEGIN(pre_message); return( _MESSAGE );

{TIME}		yylval = lex_string();	return( _TIME );
{DIGIT}+	yylval = lex_num();	return( _NUM );
{IP}		yylval = lex_string();	return( _IP );
{PORT}		yytext++;  yyleng--;  yylval = lex_num();  return( _PORT );
" "		/* ignore spaces */
{NONDIGIT}{NOSLASH}*("/"({NONDIGIT}{NOSLASH}*)?)* { yylval = lex_string();
                                                    return( _STRING ); }
<in_message>^[^\n]*$    { yylval = lex_string(); return( _STRING ); }
<pre_message>\n		BEGIN(in_message); return( _NEWLINE );
<in_message>\n          BEGIN(INITIAL); return( _NEWLINE );
\n                      return( _NEWLINE );
<<EOF>>		{
			if( yacc_block ) {
				return( _WOULDBLOCK );
			} else {
				yacc_eof = 1;
				yyterminate();
			}
		}

%%
static char rcsid[] = "$Id$";

YYSTYPE lex_string( void )
{
	char *str;

 
#ifndef USE_OBSTACKS
	str = (char *)malloc( yyleng + 1 );
	if( !str )	return( (YYSTYPE)(&yynullstr[0]) );
	memcpy( str, yytext, yyleng );
	str[yyleng] = '\0';
#else
        str = (char*)(obstack_copy0(&the_obstack, yytext, yyleng));
	if( !str )	return( (YYSTYPE)(&yynullstr[0]) );
#endif
	
	return( (YYSTYPE)str );
}

YYSTYPE lex_num( void )
{
	char *str;
	YYSTYPE ret;

	str = alloca(yyleng+1);
	if( str == 0 )	return( -1 );
        memcpy(str, yytext, yyleng);
        str[yyleng] = 0;
	ret = (YYSTYPE)atoi(str);
	return( ret );
}

YYSTYPE lex_char( void )
{
	return( (YYSTYPE)yytext[0] );
}


