PROG=chip8
SRCS=chip8.c
MAN=

CFLAGS+= `pkg-config --cflags SDL2`
LDFLAGS+= `pkg-config --libs SDL2`

.include <bsd.prog.mk>
