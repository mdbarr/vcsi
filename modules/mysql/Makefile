CC = gcc
CFLAGS =-Wall -O2 -fPIC -I../../src -I/usr/local/include

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: mysql.so

mysql.so:	mysql.o
	${CC} -shared -fPIC -o mysql.so mysql.o -lmysqlclient ${LDFLAGS}

mysql.o:	mysql.c 

clean:
	rm -f *.o *.a *.so *~ core

install: mysql.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 mysql.so $(VCSI_MOD_DIR)
