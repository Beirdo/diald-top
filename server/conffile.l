%{
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define YYSTYPE unsigned long int

#include "config.h"
#include "y.tab.h"

YYSTYPE         lex_string(void);

char            yynullstr[] = "";
int             yacc_eof;
%}

USERID	[a-zA-z][a-zA-z0-9]*
IP	[0-9]{1,3}"."[0-9]{1,3}"."[0-9]{1,3}"."[0-9]{1,3}
WS	[ \t,]
COMMENT	\#.*$

%%

{USERID}	yylval = lex_string();	return( _USERID );
{IP}		yylval = lex_string();	return( _IP );
{COMMENT}	/* ignore comments */
{WS}		/* ignore whitespace */
\n		return( _NEWLINE );
<<EOF>>		yacc_eof = 1; yyterminate();

%%
static char     rcsid[] =
    "$Id$";

YYSTYPE
lex_string(void)
{
    char           *str;

    str = (char *) malloc(yyleng + 1);
    if (!str)
	return ((YYSTYPE) (&yynullstr[0]));
    memcpy(str, yytext, yyleng);
    str[yyleng] = '\0';

    return ((YYSTYPE) str);
}
