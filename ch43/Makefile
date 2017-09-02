IDIR = ../lib
IMPL_CFLAGS = -std=c99 -D_XOPEN_SOURCE=600 \
	        -D_DEFAULT_SOURCE \
		-g -I${IDIR} \
		-pedantic \
		-Wall \
		-W \
		-Wmissing-prototypes \
		-Wno-sign-compare \
		-Wno-unused-parameter

CFLAGS = ${IMPL_CFLAGS}

TLPI_LIB = ../libtlpi.a

IMPL_LDLIBS = ${TLPI_LIB} -lm

LDLIBS = ${IMPL_LDLIBS}

pipe: pipe.o



