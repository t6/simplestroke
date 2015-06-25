SUBDIRS=	docs external src

company-clang-arguments:
	@env \
	CFLAGS='${DEFS} -I$(top_builddir)/external/sqlite3 ${CFLAGS} ${xtst_CFLAGS}' \
	python -c 'import os,shlex; s = shlex.split(os.environ["CFLAGS"]); print("\n".join(s))'

readme:
	@echo Regenerating README from man page
	@mandoc -Tascii docs/simplestroke.1 | col -bx > README

CLANG_FORMAT=clang-format

fmt:
	find src -name '*.c' -or -name '*.h' | xargs ${CLANG_FORMAT} -i -style=Mozilla

.PHONY: readme fmt
