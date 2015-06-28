SUBDIRS=	docs external src

company-clang-arguments:
	@env \
	CFLAGS='${DEFS} -I$(top_builddir)/external/sqlite3 ${CFLAGS} ${xtst_CFLAGS}' \
	python -c 'import os,shlex; s = shlex.split(os.environ["CFLAGS"]); print("\n".join(s))'

MAN_BASE=	http://www.openbsd.org/cgi-bin/man.cgi/OpenBSD-current/man%S/%N.%S?query=%N&sec=%S

README.md: docs/simplestroke.1
	@echo Regenerating README.md from man page
	@mandoc -Thtml -Ofragment -Oman="${MAN_BASE}" ${PWD}/docs/simplestroke.1 > ${@}

CLANG_FORMAT=clang-format

fmt:
	find src -name '*.c' -or -name '*.h' | xargs ${CLANG_FORMAT} -i -style=Mozilla

.PHONY: fmt
