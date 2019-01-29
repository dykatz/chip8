PROG=chip8
SRCS=chip8.c
MAN=

CFLAGS+= `pkg-config --cflags sdl2`
LDFLAGS+= `pkg-config --libs sdl2`

.include <bsd.prog.mk>
