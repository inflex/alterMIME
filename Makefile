# Wedit Makefile for project altermime
#CC=cc
#CC=ccmalloc gcc
#CFLAGS=-Wall -g
#CFLAGS=-Wall -ggdb

# Optional builds
#	ALTERMIME_PRETEXT - Allows prefixing of the email body with a file, sort of the
#								opposite of a disclaimer.
ALTERMIME_OPTIONS=-DALTERMIME_PRETEXT
#ALTERMIME_OPTIONS=
CFLAGS=-Wall -Werror -g -I. -O2 $(ALTERMIME_OPTIONS)
OBJS= strstack.o mime_alter.o ffget.o pldstr.o filename-filters.o logger.o MIME_headers.o libmime-decoders.o boundary-stack.o qpe.o


.c.o:
	${CC} ${CFLAGS} -c $*.c

all: altermime

altermime: altermime.c ${OBJS}
	${CC} ${CFLAGS} altermime.c ${OBJS} -o altermime


# Build Install
install: altermime
	strip altermime
	cp altermime /usr/local/bin
	chmod a+rx /usr/local/bin/altermime

uninstall:
	rm -f /usr/local/bin/altermime

clean:
	rm -f *.o altermime
