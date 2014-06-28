PACKAGE = bsed
VERSION = 1.0

prefix = /usr/local
bindir = ${prefix}/bin
datadir = ${prefix}/share
docdir = $(datadir)/doc
pkgdocdir = $(datadir)/doc/bsed
man1dir = $(datadir)/man/man1

CC = gcc
CFLAGS = -g -O2 -Wall
INSTALL = install

.PHONY: all doc-html

all: bsed doc-html

.c.o:
	$(CC) $(CFLAGS) -c $<

bsed: bsed.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o bsed $^

.PHONY: doc-html
doc-html: homepage/bsedman.html

homepage/bsedman.html: bsed.1
	nroff -man bsed.1 | perl man2html >homepage/bsedman.html
		
install: bsed doc-html
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -m 755 bsed $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(man1dir)
	$(INSTALL) -m 644 bsed.1 $(DESTDIR)$(man1dir)
	$(INSTALL) -d $(DESTDIR)$(pkgdocdir)
	$(INSTALL) -m 644 homepage/bsedman.html homepage/index.html $(DESTDIR)$(pkgdocdir)
	