PROG=	simplestroke
MAN=	${PROG}.1

INSTALL?=	install
CFLAGS+=        -std=${CSTD}

PACKAGES+=	libbsd
CFLAGS+=	-D_GNU_SOURCE
CFLAGS+=\
	-DHAVE_BSD_STDLIB_H \
	-DHAVE_BSD_SYS_ENDIAN_H \
	-DHAVE_BSD_STRING_H

all: ${PROG}

include Makefile.common

${PROG}: ${SRCS:.c=.o}

${MAN}.gz: ${MAN}
	gzip -cn ${<} > ${@}

install: ${PROG} ${MAN}.gz
	${INSTALL} -s -m 555  ${PROG} ${BINDIR}/${PROG}
	${INSTALL} -m 444 ${MAN}.gz  ${SHAREDIR}/man/man1

clean:
	rm -f ${PROG} ${SRCS:.c=.o}

.PHONY:	install clean
