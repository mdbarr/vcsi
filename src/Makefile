SHELL = /bin/sh

srcdir = .
top_srcdir = ..

prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${datarootdir}/info
mandir = ${datarootdir}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

CC = gcc

CFLAGS = -O2 -g -Wall -Wno-pointer-sign -Wno-unused-result -DVCSI_LIB_DIR=\"${datadir}/vcsi/lib/\" -DVCSI_MOD_DIR=\"${datadir}/vcsi/modules/\"

SOURCES = vcsi.c cont.c dynl.c env.c eval.c gc.c lex.c num.c obj.c port.c prim.c str.c sym.c vec.c net.c regexp.c thread.c srfi.c srfi-0.c srfi-1.c srfi-8.c srfi-28.c

OBJS = vcsi.o cont.o dynl.o env.o eval.o gc.o lex.o num.o obj.o port.o prim.o str.o sym.o vec.o net.o regexp.o thread.o srfi.o srfi-0.o srfi-1.o srfi-8.o srfi-28.o

HEADERS = vcsi.h srfi.h

all: libvcsi.a vcsi

vcsi:	main.o libvcsi.a
	$(CC) -rdynamic -o vcsi main.o libvcsi.a -lpthread -ldl -lreadline -lncurses -linfostd -lm  

libvcsi.a: ${OBJS} ${HEADERS}
	ar cru libvcsi.a ${OBJS}
	ranlib libvcsi.a

install: vcsi
	if test ! -d ${bindir}; then mkdir ${bindir}; fi
	/usr/bin/install -c -m 755 vcsi ${bindir}/vcsi
	if test ! -d ${libdir}; then mkdir ${libdir}; fi
	/usr/bin/install -c -m 444 libvcsi.a ${libdir}/libvcsi.a
	if test ! -d ${includedir}; then mkdir ${includedir}; fi
	if test ! -d ${includedir}/vcsi; then mkdir ${includedir}/vcsi; fi
	/usr/bin/install -c -m 444 *.h ${includedir}/vcsi/
clean:
	rm -f *.o *.a *.so *~ core vcsi
