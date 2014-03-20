CC = gcc
CFLAGS =-O2 -fPIC -I../../src

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: pipe.so

pipe.so:	pipe.o
	${CC} -shared -fPIC -o pipe.so pipe.o ${LDFLAGS}

pipe.o:	pipe.c 

clean:
	rm -f *.o *.a *.so *.*~ core

install: pipe.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 pipe.so $(VCSI_MOD_DIR)
