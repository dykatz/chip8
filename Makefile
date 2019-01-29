PROG=chip8
SRCS=chip8.c
MAN=chip8.6

CFLAGS+= `pkg-config --cflags sdl2`
LDFLAGS+= `pkg-config --libs sdl2`

README.md: chip8.6
	mandoc -T markdown $? >$@

.include <bsd.prog.mk>
