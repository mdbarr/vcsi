CC = gcc
CFLAGS =-O2 -fPIC -I../../src

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: env.so

env.so:	env.o
	${CC} -shared -fPIC -o env.so env.o ${LDFLAGS}

env.o:	env.c 

clean:
	rm -f *.o *.a *.so *.*~ core

install: env.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 env.so $(VCSI_MOD_DIR)
