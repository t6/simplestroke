PREFIX ?= /usr/local
BINDIR ?= ${PREFIX}/bin
MANDIR ?= ${PREFIX}/man/man1

# alternative SYSMOUSE
TRACKER ?= XLIB

NAME = simplestroke
CFLAGS = -g -std=c99 -Wall -Wextra -DUSE_${TRACKER}_TRACKER
LDFLAGS = -lm

PKGS = x11 xext xtst
# not using .if here to stay compatible with GNU Make
CFLAGS += `[ "${TRACKER}" = "XLIB" ] && pkg-config --cflags ${PKGS}`
LDFLAGS += `[ "${TRACKER}" = "XLIB" ] && pkg-config --libs ${PKGS}`

SRC != echo *.c

CFLAGS += -DSQLITE_THREADSAFE=1 -DSQLITE_MAX_WORKER_THREADS=0
SRC += lib/sqlite3/sqlite3.c

all: options readme ${NAME}

readme:
	@echo Regenerating README from man page
	@mandoc -Tascii ${NAME}.1 | col -bx > README

options:
	@echo ${NAME} build options:
	@echo "CC      = ${CC}"
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "TRACKER = ${TRACKER}"

fmt:
	@echo Formatting code
	@astyle --options=astylerc *.c *.h

${NAME}: ${SRC:.c=.o}
	@echo Linking ${NAME}
	@${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${SRC:.c=.o}

.c.o:
	@echo CC $<
	@${CC} ${CFLAGS} -c $< -o ${<:.c=.o}

install-options:
	@echo ${NAME} install options:
	@echo "PREFIX = ${PREFIX}"
	@echo "BINDIR = ${BINDIR}"
	@echo "MANDIR = ${MANDIR}"

install: install-options
	@echo Installing ${NAME}
	@mkdir -p ${BINDIR}
	@install -m 755 ${NAME} ${BINDIR}
	@mkdir -p ${MANDIR}
	@install -m 444 ${NAME}.1 ${MANDIR}
	@gzip -f ${MANDIR}/${NAME}.1

clean:
	rm -f ${SRC:.c=.o} ${NAME}

.PHONY: all clean ${NAME} options install install-options
