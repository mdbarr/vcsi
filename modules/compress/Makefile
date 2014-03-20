CC = gcc
CFLAGS =-O2 -fPIC -Wall -pedantic -I../../src

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: lzw.so

lzw.so:	lzw.o mod.o
	${CC} -shared -fPIC -o lzw.so lzw.o mod.o ${LDFLAGS}

lzw.o:	lzw.c lzw.h

mod.o:	mod.c lzw.h

clean:
	rm -f *.o *.a *.so *.*~ core

install: lzw.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 lzw.so $(VCSI_MOD_DIR)

