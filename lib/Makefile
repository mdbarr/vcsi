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

VCSI_LIB_DIR = ${datadir}/vcsi/lib

SCMS = srfi-0.scm srfi-1.scm srfi-8.scm

all:

clean:
	rm -f *.o *.a *.so *~ core

install:
	if test ! -d ${datadir}/vcsi; then mkdir ${datadir}/vcsi; fi
	if test ! -d ${VCSI_LIB_DIR}; then mkdir ${VCSI_LIB_DIR}; fi
	 for f in $(SCMS); do \
          /usr/bin/install -c -m 444 $$f $(VCSI_LIB_DIR)/$$f; \
        done


