include Makefile.configure

MKDIR?=		mkdir -p

CFLAGS+=	-std=c99
LDADD+=		-lm

all: simplestroke

.c.o:
	${CC} ${CPPFLAGS} -fPIC ${CFLAGS} -o $@ -c $<

simplestroke: simplestroke.o stroke.o tracker.o
	${CC} ${LDFLAGS} -o simplestroke simplestroke.o stroke.o tracker.o \
		${LDADD}

simplestroke.o: config.h stroke.h tracker.h
stroke.o: config.h stroke.h
tracker.o: config.h stroke.h tracker.h tracker_evdev.c

install:
	${MKDIR} ${DESTDIR}${BINDIR} ${DESTDIR}${MANDIR}/man1
	${INSTALL_MAN} simplestroke.1 ${DESTDIR}${MANDIR}/man1
	${INSTALL_PROGRAM} simplestroke ${DESTDIR}${BINDIR}

clean:
	@rm -f *.o simplestroke config.*.old

README.md: simplestroke.1
	mandoc -Tmarkdown simplestroke.1 > ${@}

.PHONY: all install
