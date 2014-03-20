CC = gcc
CFLAGS =-Wall -O2 -fPIC -I../../src -I/usr/local/include

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: toc2.so

toc2.so:	toc2.o
	${CC} -shared -fPIC -o toc2.so toc2.o ${LDFLAGS} 

toc2.o:	toc2.c toc2.h

clean:
	rm -f *.o *.a *.so *~ core

install: toc2.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 toc2.so $(VCSI_MOD_DIR)
