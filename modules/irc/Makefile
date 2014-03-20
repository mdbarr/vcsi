CC = gcc
CFLAGS =-O2 -fPIC -I../../src

prefix = /usr/local
datadir = ${prefix}/share
VCSI_MOD_DIR = ${prefix}/share/vcsi/modules

all: irc.so irc_v2.so quote.so definition.so unscramble.so tinyurl.so

irc.so:	irc.o
	${CC} -shared -fPIC -o irc.so irc.o ${LDFLAGS} 

irc.o:	irc.c 

irc_v2.so:	irc_v2.o
	${CC} -shared -fPIC -o irc_v2.so irc_v2.o ${LDFLAGS} 

irc_v2.o:	irc_v2.c 

quote.so: quote.o
	${CC} -shared -fPIC -o quote.so quote.o ${LDFLAGS} 

quote.o: quote.c 

definition.so: definition.o
	${CC} -shared -fPIC -o definition.so definition.o ${LDFLAGS} 

definition.o: definition.c 

unscramble.so: unscramble.o
	${CC} -shared -fPIC -o unscramble.so unscramble.o ${LDFLAGS} 

unscramble.o: unscramble.c 

tinyurl.so: tinyurl.o
	${CC} -shared -fPIC -o tinyurl.so tinyurl.o ${LDFLAGS} 

tinyurl.o: tinyurl.c 

clean:
	rm -f *.o *.a *.so *~ core

install: irc.so quote.so definition.so unscramble.so tinyurl.so
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_MOD_DIR}; then mkdir ${VCSI_MOD_DIR}; fi
	/usr/bin/install -c -m 444 irc.so $(VCSI_MOD_DIR)
	/usr/bin/install -c -m 444 irc_v2.so $(VCSI_MOD_DIR)
	/usr/bin/install -c -m 444 quote.so $(VCSI_MOD_DIR)
	/usr/bin/install -c -m 444 definition.so $(VCSI_MOD_DIR)
	/usr/bin/install -c -m 444 unscramble.so $(VCSI_MOD_DIR)
	/usr/bin/install -c -m 444 tinyurl.so $(VCSI_MOD_DIR)
