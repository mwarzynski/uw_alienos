# The AlienOS operating system

## Binary format

It seems that the aliens are very advanced – they use 64-bit x86 architecture and ELF format.

Alien programs are statically linked `ELF` files of the type `ET_EXEC`. They do not use dynamic memory allocation and are always loaded at addresses in the range `0x31337000 .. 0x80000000`. When starting the program, `rsp` should contain the top address of some sensible stack, and the value of other registers is not significant.

In addition to the standard `PT_LOAD` segments, these files may also contain a segment of the special type `PT_PARAMS (0x60031337)`. This segment, if it exists, is used to pass parameters from the operating system. Parameters for the program are always of type int and are passed in binary form (4 bytes, little-endian). The size of the `PT_PARAMS` segment indicates how many parameters a given program needs (`p_memsz / 4`). Before the program starts, the operating system places parameter values in this segment. If the program does not have such a segment, it means that it does not accept parameters.

## System calls

System calls are made using the syscall instruction. The number of the system call is passed in the register rax, the return value from the call is also in the register rax, and the parameters are in the registers `rdi, rsi, rdx, r10` (in that order). The following known system calls exist:

 - `0`: `void noreturn end(int status)`
 - `1`: `uint32_t getrand()`
 - `2`: `int getkey()`
 - `3`: `void print(int x, int y, uint16_t *chars, int n)`
 - `4`: `void setcursor(int x, int y)`

### end

Ends the program with the given exit code. The exit code should be in the range 0-63.

### getrand

Returns a random 32-bit number.

### getkey

Waits for pressing a key on the keyboard and returns his code. We know of the following keycodes on alien keyboards:

 - `0x0a`: enter
 - `0x20-0x7e`: as in ASCII
 - `0x80`: up arrow
 - `0x81`: left arrow
 - `0x82`: down arrow
 - `0x83`: right arrow

There is no echo in the AlienOS system – the keys pressed are not automatically printed on the terminal.

### print

Print the given characters to the screen in the given (x, y) position. Aliens use 80×24 text terminals. x means the column index, counted from 0 on the left. y means the row index, counted from 0 from the top of the terminal. chars is a pointer to n characters to be printed in the given line, starting from the given column and going to the right. Each character is a 16-bit number with the following fields:

 - bits 0-7: character code in ASCII (always in the range of 0x20 ... 0x7e – the aliens are not advanced enough yet to invent Unicode).
 - bits 8-11: character color:
    - 0: black
    - 1: blue
    - 2: green
    - 3: turquoise
    - 4: red
    - 5: pink
    - 6: yellow
    - 7: light gray
    - 8: dark gray
    - 9: blue (bright)
    - 10: green (bright)
    - 11: turquoise (bright)
    - 12: red (bright)
    - 13: pink (bright)
    - 14: yellow (bright)
    - 15: white
 - bits 12-15: don’t seem to be used for anything

Calling `print` does not change the cursor position on the terminal.

### setcursor

Moves the cursor in the terminal to the given coordinates.

