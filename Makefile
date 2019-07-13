LOCALBASE?=		/usr/local
PREFIX?=		/usr/local
BINDIR?=		${PREFIX}/bin
MANDIR?=		${PREFIX}/man
DESTDIR?=
BSD_INSTALL_PROGRAM?=	install -s -m 555
BSD_INSTALL_MAN?=	install -m 444
PKGCONF?=		pkg-config

PKGS=
.if defined(USE_X11)
PKGS+=		x11 xtst
CFLAGS+=	-DUSE_X11
.endif
.if defined(USE_EVDEV)
CFLAGS+=	-DUSE_EVDEV -I${LOCALBASE}/include
.endif

PKG_CFLAGS=	`${PKGCONF} --cflags ${PKGS}`
PKG_LDFLAGS=	`${PKGCONF} --libs ${PKGS}`

CFLAGS+=	${PKG_CFLAGS}
LDFLAGS+=	-pthread -lm
LDFLAGS+=	${PKG_LDFLAGS}

CFLAGS+=	-std=c99 -I.
CFLAGS+=        -Wall -Wextra -Wshadow

.if defined(USE_X11)
all: simplestroke simplestroke-daemon
.elif defined(USE_EVDEV)
all: simplestroke
.else
all:
	@echo "You must define which backend to build: USE_X11, USE_EVDEV, or both"
	@false
.endif

simplestroke: simplestroke.o stroke.o tracker_x11.o tracker_evdev.o
	${CC} -o ${@} ${LDFLAGS} simplestroke.o stroke.o tracker_x11.o tracker_evdev.o

simplestroke-daemon: simplestroke-daemon.o
	${CC} -o ${@} ${LDFLAGS} simplestroke-daemon.o

install: all
	mkdir -p ${DESTDIR}${BINDIR}
	${BSD_INSTALL_PROGRAM} simplestroke ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man1
	${BSD_INSTALL_MAN} simplestroke.1 ${DESTDIR}${MANDIR}/man1
.if defined(USE_X11)
	${BSD_INSTALL_PROGRAM} simplestroke-daemon ${DESTDIR}${BINDIR}
	${BSD_INSTALL_MAN} simplestroke-daemon.1 ${DESTDIR}${MANDIR}/man1
.endif

clean:
	rm -f simplestroke simplestroke-daemon *.o

README.md:
	mandoc -Tmarkdown simplestroke.1 > ${@}

.PHONY:	README.md install clean
