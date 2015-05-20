AC_INIT([Simplestroke],
        [master],
        [t@tobik.me],
        [simplestroke],
        [https://github.com/t6/simplestroke],
        [t@tobik.me])
AC_CONFIG_SRCDIR([configure.ac])
AM_INIT_AUTOMAKE([1.11 foreign -Wno-portability no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])

AC_PROG_CC_C99
LT_INIT()
AC_CONFIG_MACRO_DIR([m4])
AC_C_CONST
AC_TYPE_SIZE_T
# AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST
case $host_os in
linux*)
        CFLAGS="$CFLAGS -D_GNU_SOURCE"
;;
esac
AC_CHECK_FUNC([asprintf])

AC_ARG_WITH([sysmouse-tracker], AS_HELP_STRING(
        [--with-sysmouse-tracker], [Use the sysmouse mouse tracker.
        By default the Xlib mouse tracker is used.]),
        [],
        [with_sysmouse_tracker=no])
AS_IF([test "x$with_sysmouse_tracker" != xno],
            [AC_DEFINE([USE_SYSMOUSE_TRACKER], [], [using sysmouse tracker])],
            [AC_DEFINE([USE_XLIB_TRACKER], [], [using XLib mouse tracker])
             PKG_CHECK_MODULES([xtst], [x11 xext xtst])])

AC_CHECK_FUNC([strlcpy],
        [],
        [PKG_CHECK_MODULES([libbsd], [libbsd], [CFLAGS="$CFLAGS $libbsd_CFLAGS"
                                                LIBS="$LIBS $libbsd_LIBS"])])

AC_CHECK_HEADERS_ONCE([bsd/string.h])
AC_CHECK_LIB([m], [atan2])
AC_CHECK_LIB([m], [hypot])

AC_CONFIG_FILES([
Makefile
docs/Makefile
src/Makefile
external/Makefile
])
AC_OUTPUT
