CC = cc

# pkg-config packages
PKGS = x11 xext xtst sqlite3
CFLAGS = -g -std=c99 -Wall -Wextra
CFLAGS += `pkg-config --cflags ${PKGS}`
LDFLAGS = -lm `pkg-config --libs ${PKGS}` -lxo

SRC = recorder-x11.c \
	db.c \
	stroke.c \
	util.c \
	simplestroke.c \
	simplestroke_new.c \
	simplestroke_detect.c \
	simplestroke_export.c

all: options simplestroke

options:
	@echo simplestroke build options:
	@echo "CC      = ${CC}"
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"

fmt:
	@echo formatting code
	@astyle --options=astylerc ${SRC} *.h

simplestroke: ${SRC:.c=.o}
	@echo Linking simplestroke
	@${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${SRC:.c=.o}

.c.o:
	@echo CC $<
	@${CC} ${CFLAGS} -c $<

clean:
	rm -f ${SRC:.c=.o} simplestroke

.PHONY: all clean simplestroke options
