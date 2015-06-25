noinst_HEADERS=\
	db.h \
	stroke.h \
	util.h

bin_PROGRAMS=	simplestroke
simplestroke_CFLAGS=\
	-std=c99 -Wall -Wextra \
	$(xtst_CFLAGS) \
	-I$(top_builddir)/external/sqlite3
simplestroke_LDADD=\
	$(xtst_LIBS) \
	$(top_builddir)/external/libsqlite3_static.la
simplestroke_SOURCES=\
	db.c \
	recorder-sysmouse.c \
	recorder-x11.c \
	simplestroke.c \
	simplestroke_delete.c \
	simplestroke_detect.c \
	simplestroke_export.c \
	simplestroke_list.c \
	simplestroke_new.c\
	stroke.c \
	util.c
