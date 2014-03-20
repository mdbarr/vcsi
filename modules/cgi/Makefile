CC = gcc
CFLAGS =-O2 -fPIC -I../../src

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: cgi.so

cgi.so:	cgi.o
	${CC} -shared -fPIC -o cgi.so cgi.o ${LDFLAGS}

cgi.o:	cgi.c 

clean:
	rm -f *.o *.a *.so *.*~ core

install: cgi.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 cgi.so $(VCSI_MOD_DIR)
