#ifndef _ALIENOS_H
#define _ALIENOS_H 1

#define _GNU_SOURCE

#include <elf.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/user.h>

#include <asm/ptrace-abi.h>

#include <linux/ptrace.h>


#define SYS_getrandom 318

// Character of aliens.
struct alien_char {
    uint8_t c;
    uint8_t color;
} typedef alien_char;


// Alien colors.
#define ALIEN_COLOR_BLACK 0
#define ALIEN_COLOR_BLUE 1
#define ALIEN_COLOR_GREEN 2
#define ALIEN_COLOR_TURQUOISE 3
#define ALIEN_COLOR_RED 4
#define ALIEN_COLOR_PINK 5
#define ALIEN_COLOR_YELLOW 6
#define ALIEN_COLOR_GREY_LIGHT 7
#define ALIEN_COLOR_GREY_DARK 8
#define ALIEN_COLOR_BRIGHT_BLUE 9
#define ALIEN_COLOR_BRIGHT_GREEN 10
#define ALIEN_COLOR_BRIGHT_TURQUOISE 11
#define ALIEN_COLOR_BRIGHT_RED 12
#define ALIEN_COLOR_BRIGHT_PINK 13
#define ALIEN_COLOR_BRIGHT_YELLOW 14
#define ALIEN_COLOR_WHITE 15


// Key presses representation as numbers.
// Range for ASCII: 0x20 - 0x7e.
#define ALIEN_KEY_UP 0x80
#define ALIEN_KEY_LEFT 0x81
#define ALIEN_KEY_DOWN 0x82
#define ALIEN_KEY_RIGHT 0x83
#define ALIEN_KEY_ENTER 0x0a
#define ALIEN_KEY_ASCII_MIN 0x20
#define ALIEN_KEY_ASCII_MAX 0x7e


// Syscall hints:
// The number of the system call is passed in the register rax.
// The return value from the call is also in the register rax,

#define ALIEN_SYSCALL_END 0
// void noreturn end(int status)
//                   rdi

#define ALIEN_SYSCALL_GETRAND 1
// uint32_t getrand()
//

#define ALIEN_SYSCALL_GETKEY 2
// int getkey()
//

#define ALIEN_SYSCALL_PRINT 3
// void print(int x, int y, uint16_t *chars, int n)
//            rdi  , rsi  , rdx            , r10

#define ALIEN_SYSCALL_SETCURSOR 4
// void setcursor(int x, int y)
//                rdi  , rsi

// Define range for alien's exit code.
#define ALIEN_END_CODE_MIN 0
#define ALIEN_END_CODE_MAX 63

// PT - segment for parameters passed from the operating system.
// Parameters are always of type int and are passed in binary form
// (4 bytes, little-endian).
// The size of the `PT_PARAMS` segment indicates how many parameters
// a given program needs (`p_memsz / 4`).
// Before the program starts, the operating system places parameter values
// in this segment. If the program does not have such a segment,
// it means that it does not accept parameters.
#define ALIEN_PT_PARAMS 0x60031337

// Memory allocation allowed range address.
#define ALIEN_LOAD_ADDR_MIN 0x31337000
#define ALIEN_LOAD_ADDR_MAX 0x80000000

// Terminal - alien size
#define ALIEN_TERMINAL_WIDTH_MIN 80
#define ALIEN_TERMINAL_HEIGHT_MIN 24


// Descriptor to emulated program.
FILE *fp;
// Pointer to the beginning of loaded file.
uint8_t *file;
// Size of loaded file (in bytes).
long file_size;

// Pointer to file ELF header.
Elf64_Ehdr *elf_header;

// Pointers to program headers.
Elf64_Phdr **program_headers;
// len: elf_header->e_phnum


// Child PID.
pid_t alien_child;

// Registers from the child while the syscall stop.
typedef struct user_regs_struct registers;


// Terminal - functions.
int alien_terminal_init();
void alien_terminal_goto(int x, int y);
void alien_terminal_show(alien_char *s, int n);
int alien_terminal_getsize(struct winsize *w);
void alien_terminal_cleanup();
// Temrminal - current position of terminal cursor.
int terminal_x;
int terminal_y;

// Load file and accordingly parse ELF structures.
// Also, check parameters.
// If any error occurs, alien_init will return 127.
int alien_init(int argc, char *argv[]);
// Clean up the memory (should be called when the process ends).
int alien_init_cleanup();

// Emulate AlienOS syscalls.
// It modifies registers which later should be set
// as to allow setting return value.
int alien_emulate();

// Execute given program.
// Also, sets up ptrace.
int alien_exec();

// Exit the alien program 'gracefully' (SIGKILL).
void alien_exit(int code);

#endif // _ALIENOS_H
