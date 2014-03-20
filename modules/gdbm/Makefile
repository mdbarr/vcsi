CC = gcc
CFLAGS =-Wall -O2 -fPIC -I../../src -I/usr/local/include

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: gdbm.so

gdbm.so:	gdbm.o
	${CC} -shared -fPIC -o gdbm.so gdbm.o -lgdbm ${LDFLAGS}

gdbm.o:	gdbm.c 

clean:
	rm -f *.o *.a *.so *~ core

install: gdbm.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 gdbm.so $(VCSI_MOD_DIR)
