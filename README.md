CHIP8(6) - Games Manual

# NAME

**chip8** - a simple CHIP-8 system emulator

# SYNOPSIS

**chip8**
*game*

# DESCRIPTION

The game
**chip8**
plays a CHIP-8 ROM. It will appear in its own window.

# CONTROLS

CHIP-8 uses a 16 key keypad that does not correspond to the way a normal
keyboard is layed out. The keys are the hexedecimal digits (0-9, A-F), and
layed out as follows:

	C D E F  
	8 9 A B  
	4 5 6 7  
	0 1 2 3

These are mapped to the rightmost 16 keys on the standard QWERTY keyboard:

	1 2 3 4  
	Q W E R  
	A S D F  
	Z X C V

Unfortunately, this is hardcoded at compile time.

# SEE ALSO

intro(6)

# AUTHORS

This
**chip8**
emulator implementation was written by Dylan Katz.

# CAVEATS

This does not ship with any binaries. One may find them on the web.

Mac OS X 10.14 - November 23, 2019
