AUTOMAKE_OPTIONS= subdir-objects

noinst_HEADERS=	sqlite3/sqlite3.h
noinst_LTLIBRARIES=	libsqlite3_static.la

libsqlite3_static_la_SOURCES=	sqlite3/sqlite3.c
libsqlite3_static_la_CFLAGS=\
	-DSQLITE_THREADSAFE=0 \
	-DSQLITE_OMIT_LOAD_EXTENSION=1 \
	-DNDEBUG \
	-static
libsqlite3_static_la_LDFLAGS=	-all-static
