noinst_HEADERS=\
	db.h \
	stroke.h \
	util.h

bin_PROGRAMS=	simplestroke
simplestroke_CFLAGS=\
	$(x11_CFLAGS) $(xext_CFLAGS) $(xtst_CFLAGS) \
	-I$(top_builddir)/external/sqlite3
simplestroke_LDADD=\
	$(x11_LIBS) $(xext_LIBS) $(xtst_LIBS) \
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
