# $Id$
#
# Makefile for diald-top package
# 
# (c) 1995-97 Gavin J. Hurlbut
# gjhurlbu@beirdo.uplink.on.ca
#

include ../Makefile.def

OBJS = diald-top-server.o port-daemon.o port-queue.o fifo-server.o lex.yy.o \
	y.tab.o authenticate.o selection.o
DESTBINS = ${SERVERBINS:%=../%}

../% : %
	cp ${@F} $@

all:	${DESTBINS}


diald-top-server: ${OBJS}
	gcc ${LDFLAGS} -o $@ $+ ${SERVERLIB}

lex.yy.c:	y.tab.h

lex.yy.c:	conffile.l
	${LEX} $<

y.tab.h y.tab.c:	conffile.y
	${YACC} $<


distclean clean:
	-rm -f ${OBJS} ${SERVERBINS} core


squeakyclean:	clean
	-rm -f y.tab.c y.tab.h y.output lex.yy.c
	
install:
	${MAKE} -C .. install-server
	

.c.o:
	${CC} ${CFLAGS} ${SERVERINCLUDES} ${SERVERDEFINES} -c $< -o $@

