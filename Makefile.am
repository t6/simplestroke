SUBDIRS=	docs external src

readme:
	@echo Regenerating README from man page
	@mandoc -Tascii docs/simplestroke.1 | col -bx > README

fmt:
	@find src -name '*.c' -or -name '*.h' | xargs astyle --options=astylerc

.PHONY: readme fmt
