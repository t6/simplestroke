PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man
DESTDIR?=	
INSTALL?=	install
PKGCONF?=	pkg-config

PROG=	simplestroke
MAN=	simplestroke.1

PKGS+=	x11 xtst

PKG_CFLAGS=	`${PKGCONF} --cflags ${PKGS}`
PKG_LDFLAGS=	`${PKGCONF} --libs ${PKGS}`

CFLAGS+=	${PKG_CFLAGS}
LDFLAGS+=	-pthread -lm
LDFLAGS+=	${PKG_LDFLAGS}

CFLAGS+=	-std=c99 -I.
CFLAGS+=        -Wall -Wextra -Wshadow

SRCS+=\
	simplestroke.c \
	stroke.c \
	tracker.c

all: ${PROG}

${PROG}: ${SRCS:.c=.o}
	${CC} -o ${@} ${LDFLAGS} ${SRCS:.c=.o}

install: ${PROG} ${MAN}
	mkdir -p ${DESTDIR}${BINDIR}
	${INSTALL} -s -m 555  ${PROG} ${DESTDIR}${BINDIR}/${PROG}
	mkdir -p ${DESTDIR}${MANDIR}/man1
	${INSTALL} -m 444 ${MAN} ${DESTDIR}${MANDIR}/man1/${MAN}

clean:
	rm -f ${PROG} ${SRCS:.c=.o}

MAN_URL=	https://www.freebsd.org/cgi/man.cgi?query=%N&sektion=%S&apropos=0&manpath=FreeBSD+10.3-RELEASE+and+Ports

README.md: ${MAN}
	mandoc -Thtml -Ofragment -Oman="${MAN_URL}" ${MAN} > ${@}

.PHONY:	install clean
