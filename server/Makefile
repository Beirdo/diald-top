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
	${CP} ${@F} $@

all:	${DESTBINS}


diald-top-server: ${OBJS}
	${CC} ${LDFLAGS} -o $@ $+ ${SERVERLIB}

lex.yy.c:	y.tab.h

lex.yy.c:	conffile.l
	${LEX} $<

y.tab.h y.tab.c:	conffile.y
	${YACC} $<


distclean clean:
	-${RM} ${OBJS} ${SERVERBINS} core


squeakyclean:	clean
	-${RM} y.tab.c y.tab.h y.output lex.yy.c
	
install:
	${MAKE} -C .. install-server
	

.c.o:
	${CC} ${CFLAGS} ${SERVERINCLUDES} ${SERVERDEFINES} -c $< -o $@

