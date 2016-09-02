PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man
DESTDIR?=
INSTALL?=	install
PKGCONF?=	pkg-config

PKGS+=	x11 xtst

PKG_CFLAGS=	`${PKGCONF} --cflags ${PKGS}`
PKG_LDFLAGS=	`${PKGCONF} --libs ${PKGS}`

CFLAGS+=	${PKG_CFLAGS}
LDFLAGS+=	-pthread -lm
LDFLAGS+=	${PKG_LDFLAGS}

CFLAGS+=	-std=c99 -I.
CFLAGS+=        -Wall -Wextra -Wshadow

all: simplestroke simplestroke-daemon

simplestroke: simplestroke.o stroke.o tracker.o
	${CC} -o ${@} ${LDFLAGS} simplestroke.o stroke.o tracker.o

simplestroke-daemon: simplestroke-daemon.o
	${CC} -o ${@} ${LDFLAGS} simplestroke-daemon.o

install: ${PROG} ${MAN}
	mkdir -p ${DESTDIR}${BINDIR}
	${INSTALL} -s -m 555 simplestroke ${DESTDIR}${BINDIR}/
	${INSTALL} -s -m 555 simplestroke-daemon ${DESTDIR}${BINDIR}/
	mkdir -p ${DESTDIR}${MANDIR}/man1
	${INSTALL} -m 444 simplestroke.1 ${DESTDIR}${MANDIR}/man1/
	${INSTALL} -m 444 simplestroke-daemon.1 ${DESTDIR}${MANDIR}/man1/

clean:
	rm -f simplestroke simplestroke-daemon *.o

MAN_URL=	https://man.freebsd.org/%N(%S)

README.md:
	mandoc -Thtml -Ofragment -Oman="${MAN_URL}" simplestroke.1 > ${@}

.PHONY:	README.md install clean
