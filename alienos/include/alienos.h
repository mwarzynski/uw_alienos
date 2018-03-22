#ifndef _ALIENOS_H
#define _ALIENOS_H 1

#include <elf.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/random.h>

#include <asm/ptrace-abi.h>


// Key presses representation as numbers.
// Range for ASCII: 0x20 - 0x7e.
#define KEY_UP 0x80
#define KEY_LEFT 0x81
#define KEY_DOWN 0x82
#define KEY_RIGHT 0x83
#define KEY_ENTER 0x0

// Syscall hints:
// The number of the system call is passed in the register rax.
// The return value from the call is also in the register rax,

#define SYSCALL_END 0
// void noreturn end(int status)
//                   rdi

#define SYSCALL_GETRAND 1
// uint32_t getrand()
//

#define SYSCALL_GETKEY 2
// int getkey()
//

#define SYSCALL_PRINT 3
// void print(int x, int y, uint16_t *chars, int n)
//            rdi  , rsi  , rdx            , r10

#define SYSCALL_SETCURSOR 4
// void setcursor(int x, int y)
//                rdi  , rsi


// PT - segment for parameters passed from the operating system.
// Parameters are always of type int and are passed in binary form (4 bytes, little-endian).
// The size of the `PT_PARAMS` segment indicates how many parameters a given program
// needs (`p_memsz / 4`).
// Before the program starts, the operating system places parameter values in this segment.
// If the program does not have such a segment, it means that it does not accept parameters.
#define PT_PARAMS 0x60031337


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

// Pointers to section headers.
Elf64_Shdr **section_headers;
// len: elf_header->e_shnum

// Program header of type PT_PARAMS.
Elf64_Phdr *parameters_header;


// Child PID.
pid_t child;

// Registers from the child while the syscall stop.
typedef struct user_regs_struct registers;


// Terminal functions
void alien_terminal_goto(int x, int y);
void alien_terminal_show(char *s, int n);

void alien_setup_signal_handler();

// Load file and accordingly parse ELF structures.
// Also, check parameters.
// If any error occurs, alien_init will return 127.
void alien_init(int argc, char *argv[]);
// Clean up the memory.
void alien_init_cleanup();

// Emulate AlienOS syscalls.
// It modifies registers which later should be set
// as to allow setting return value.
int alien_emulate(registers *regs);

// Execute given program.
// Also, sets up ptrace.
void alien_exec();

// Exit the alien program 'gracefully' (SIGKILL).
void alien_exit(int code);

#endif // _ALIENOS_H
