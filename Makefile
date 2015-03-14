CC = cc

# pkg-config packages
PKGS = x11 xext xtst
CFLAGS = -g -std=c99 -Wall -Wextra
CFLAGS += `pkg-config --cflags ${PKGS}`
LDFLAGS = -lm `pkg-config --libs ${PKGS}`

SRC = \
	recorder-x11.c \
	db.c \
	stroke.c \
	util.c \
	simplestroke.c \
	simplestroke_new.c \
	simplestroke_detect.c \
	simplestroke_export.c \
	simplestroke_list.c

CFLAGS += \
	-DSQLITE_THREADSAFE=1 \
	-DSQLITE_MAX_WORKER_THREADS=0

SRC += \
	lib/sqlite3/sqlite3.c

all: options readme simplestroke

readme:
	@echo regenerating README from man page
	@mandoc -Tascii simplestroke.1 | col -bx > README

options:
	@echo simplestroke build options:
	@echo "CC      = ${CC}"
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"

fmt:
	@echo formatting code
	@astyle --options=astylerc *.c *.h

simplestroke: ${SRC:.c=.o}
	@echo Linking simplestroke
	@${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${SRC:.c=.o}

.c.o:
	@echo CC $<
	@${CC} ${CFLAGS} -c $< -o ${<:.c=.o}

clean:
	rm -f ${SRC:.c=.o} simplestroke

.PHONY: all clean simplestroke options
