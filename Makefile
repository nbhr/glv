all: srcdir

srcdir:
	@(cd src; ${MAKE})

clean:
	@(cd src; ${MAKE} clean)

