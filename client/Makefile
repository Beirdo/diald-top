# $Id$
#
# Makefile for diald-top package
# 
# (c) 1995-97 Gavin J. Hurlbut
# gjhurlbu@beirdo.uplink.on.ca
#

include ../Makefile.def

OBJS = diald-top.o pipe.o resolve.o screen.o lex.yy.o y.tab.o parser.o 
DESTBINS = ${CLIENTBINS:%=../%}

../% : %
	${CP} ${@F} $@

all:	${DESTBINS}

diald-top:	${OBJS}
	${CC} $(LDFLAGS) -o $@ $+ ${CLIENTLIB}

distclean clean:
	-${RM} ${OBJS} ${CLIENTBINS} tester core

squeakyclean:	clean
	-${RM} y.tab.c y.tab.h y.output lex.yy.c

install:
	${MAKE} -C .. install-client
	

parser.c lex.yy.c:	y.tab.h

lex.yy.c:	monitor.l
	${LEX} $<

y.tab.h y.tab.c:	monitor.y
	${YACC} $<


tester:		tester.o lex.yy.o y.tab.o tparser.o
	${CC} -s -o $@ $+ -lfl

tparser.o:	parser.c
	${CC} ${CFLAGS} ${CLIENTINCLUDES} ${CLIENTDEFINES} -DTESTER -c $< -o $@


.c.o:
	${CC} ${CFLAGS} ${CLIENTINCLUDES} ${CLIENTDEFINES} -c $< -o $@

