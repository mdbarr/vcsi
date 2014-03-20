CC = gcc
CFLAGS =-O2 -fPIC -I../../src

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: infostd-speller.so infostd-stemmer.so

infostd-speller.so:	infostd-speller.o
	${CC} -shared -fPIC -o infostd-speller.so infostd-speller.o ${LDFLAGS}

infostd-speller.o:	infostd-speller.c 

infostd-stemmer.so:	infostd-stemmer.o
	${CC} -shared -fPIC -o infostd-stemmer.so infostd-stemmer.o ${LDFLAGS}

infostd-stemmer.o:	infostd-stemmer.c 


clean:
	rm -f *.o *.a *.so *.*~ core

install: infostd-speller.so infostd-stemmer.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 infostd-speller.so $(VCSI_MOD_DIR)
	/usr/bin/install -c -m 444 infostd-stemmer.so $(VCSI_MOD_DIR)
