CC = gcc
CFLAGS =-O2 -fPIC -I../../src

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: http.so

http.so:	http.o
	${CC} -shared -fPIC -o http.so http.o ${LDFLAGS}

http.o:	http.c 

clean:
	rm -f *.o *.a *.so *.*~ core

install: http.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 http.so $(VCSI_MOD_DIR)
