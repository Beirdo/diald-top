# $Id$
#
# Makefile for diald-top package
# 
# (c) 1995-96 Gavin J. Hurlbut
# gjhurlbu@beirdo.uplink.on.ca
#

include Makefile.def

# Default is to make all
all::

DIRS = client server

all clean distclean squeakyclean::
	for i in ${DIRS}; do ${MAKE} $@ -C $$i ; done

.PHONY:	${DIRS}

${DIRS}:
	${MAKE} -C $@

clean squeakyclean::
	-rm -f diald-top diald-top-server core

install:	install-client install-server

install-client:	client
	${INSTALL} -o root -g root -m 755 -d ${CLIENT_INSTALL_DIR}
	${INSTALL} -o root -g root -m 755 -d ${MAN_DIR}
	${INSTALL} -o ${OWNER} -g ${GROUP} ${INSTALLFLAGS} -m ${CLIENTMODE} \
		   ${CLIENTBINS} ${CLIENT_INSTALL_DIR}
	${INSTALL} -m 644 diald-top.8 ${MAN_DIR}

install-server:	server
	${INSTALL} -o root -g root -m 755 -d ${SERVER_INSTALL_DIR}
	${INSTALL} -o ${OWNER} -g ${GROUP} ${INSTALLFLAGS} -m ${SERVERMODE} \
		   ${SERVERBINS} ${SERVER_INSTALL_DIR}

