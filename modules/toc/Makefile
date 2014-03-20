CC = gcc
CFLAGS =-Wall -O2 -fPIC -I../../src -I/usr/local/include

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: toc.so

toc.so:	toc.o
	${CC} -shared -fPIC -o toc.so toc.o ${LDFLAGS} 

toc.o:	toc.c toc.h

clean:
	rm -f *.o *.a *.so *~ core

install: toc.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 toc.so $(VCSI_MOD_DIR)
