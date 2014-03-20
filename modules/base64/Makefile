CC = gcc
CFLAGS =-O2 -fPIC -I../../src

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: base64.so

base64.so:	base64.o
	${CC} -shared -fPIC -o base64.so base64.o ${LDFLAGS}

base64.o:	base64.c 

clean:
	rm -f *.o *.a *.so *.*~ core

install: base64.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 base64.so $(VCSI_MOD_DIR)
