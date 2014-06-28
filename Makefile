VERSION=1.0
WORLDWIDE=-worldwide
TYPES = `machines gnutypes|xargs|tr ' ' ','`
default:
	@echo "no default target"

build:
	expmake -parallel $(WORLDWIDE) version=$(VERSION) executableTypes=$(TYPES)

provide:
	expprovide $(WORLDWIDE) version=$(VERSION) executableTypes=$(TYPES)

preview:
	expprovide $(WORLDWIDE) version=$(VERSION) executableTypes=$(TYPES) -preview

bsed:  bsed.c
	$(CC) -O -o bsed bsed.c

manpage:
	nroff -man bsed.1 | /opt/exp/perl/bin/man2html -topm 6 -botm 6 \
		-compress -title "bsed man page" >homepage/bsedman.html
