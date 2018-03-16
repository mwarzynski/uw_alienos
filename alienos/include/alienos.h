
#ifndef _ALIENOS_H
#define _ALIENOS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <elf.h>

// Key presses representation as numbers.
// Range for ASCII: 0x20 - 0x7e.
#define KEY_UP 0x80
#define KEY_LEFT 0x81
#define KEY_DOWN 0x82
#define KEY_RIGHT 0x83
#define KEY_ENTER 0x0

// PT - segment for parameters passed from the operating system.
// Parameters are always of type int and are passed in binary form (4 bytes, little-endian).
// The size of the `PT_PARAMS` segment indicates how many parameters a given program
// needs (`p_memsz / 4`).
// Before the program starts, the operating system places parameter values in this segment.
// If the program does not have such a segment, it means that it does not accept parameters.
#define PT_PARAMS 0x60031337


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

// Load file and accordingly parse ELF structures.
// Also, check parameters.
// If any error occurs, alien_init will return 127.
int alien_init(int argc, char *argv[]);

#endif // _ALIENOS_H
