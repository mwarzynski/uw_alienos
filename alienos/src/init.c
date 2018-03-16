#include "init.h"


void alien_init_loadfile(char *filename) {
    file = NULL;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fputs("init_loadfile: File descriptor is NULL.\n", stderr);
        exit(127);
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        fputs("init_loadfile: Seeking to the end of file error.\n", stderr);
        exit(127);
    }

    file_size = ftell(fp);
    if (file_size == -1) {
        fputs("init_loadfile: Could not determine file size.\n", stderr);
        exit(127);
    }

    file = malloc(sizeof(char) * (file_size + 1));

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        fputs("init_loadfile: Could not seek to the file beggining.\n", stderr);
        exit(127);
    }

    size_t len = fread(file, sizeof(char), file_size, fp);
    if (ferror(fp) != 0 ) {
        fputs("init_loadfile: Error reading file.\n", stderr);
        exit(127);
    }

    file[len++] = '\0';

    fclose(fp);
}

void alien_init_program_headers() {
    program_headers = malloc(sizeof(Elf64_Phdr*) * elf_header->e_phnum);
    if (program_headers == NULL) {
        // malloc failed to allocate memory
        fputs("init_program_headers: could not allocate memory.\n", stderr);
        exit(127);
    }

    Elf64_Off offset = elf_header->e_phoff;
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        program_headers[i] = (Elf64_Phdr*)(file + offset + i*(elf_header->e_phentsize));
    }
}

void alien_init_section_headers() {
    section_headers = malloc(sizeof(Elf64_Shdr*) * elf_header->e_shnum);
    if (section_headers == NULL) {
        // malloc failed to allocate memory
        fputs("init_section_headers: could not allocate memory.\n", stderr);
        exit(127);
    }

    Elf64_Off offset = elf_header->e_shoff;
    for (size_t i = 0; i < elf_header->e_shnum; i++) {
        section_headers[i] = (Elf64_Shdr*)(file + offset + i*(elf_header->e_shentsize));
    }
}

void alien_init_parse_elf() {
    // ELF header - for 64-bit architecture is 64 bytes long.
    if (file_size < 64) {
        fputs("init_parse_elf: It's not an ELF file.\n", stderr);
        exit(127);
    }

    elf_header = (Elf64_Ehdr*)file;

    for (size_t i = 0; i < SELFMAG; i++) {
        if (file[i] != elf_header->e_ident[i]) {
            fputs("init_parse_elf: ELF header - invalid magic number.\n", stderr);
            exit(127);
        }
    }

    // ELF header - check architecture.
    if (elf_header->e_type != ELFCLASS64) {
        fputs("init_parse_elf: ELF header - not a 64-bit format.\n", stderr);
        exit(127);
    }

    // ELF header - check file type.
    if (elf_header->e_type != ET_EXEC) {
        fputs("init_parse_elf: ELF header - invalid type of file.\n", stderr);
        exit(127);
    }

    // ELF header - check endianess.
    if ((elf_header->e_machine >> EI_DATA) != ELFDATA2LSB) {
        fputs("init_parse_elf: ELF header - not a little-endian.\n", stderr);
        exit(127);
    }

    // ELF header - check architecture.
    if (elf_header->e_machine != EM_X86_64) {
        fputs("init_parse_elf: ELF header - invalid architecture\n", stderr);
        exit(127);
    }

    // ELF header - check version.
    if (elf_header->e_version != EV_CURRENT) {
        fputs("init_parse_elf: ELF header - invalid version.\n", stderr);
        exit(127);
    }
}

Elf64_Xword alien_init_parse_params() {
    parameters_header = NULL;
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        if (program_headers[i]->p_type == PT_PARAMS) {
            parameters_header = program_headers[i];
            break;
        }
    }
    if (parameters_header == NULL) {
        return 0;
    }

    return parameters_header->p_memsz / 4;
}

int alien_init(int argc, char *argv[]) {
    if (argc < 2) {
        fputs("init: You must specify program to execute.\n", stderr);
        exit(127);
    }

    // Load file content to memory.
    alien_init_loadfile(argv[1]);

    // Parse ELF header file.
    alien_init_parse_elf();

    // Parse program headers.
    alien_init_program_headers();

    // ELF header - parse section header entries.
    alien_init_section_headers();

    Elf64_Xword params = alien_init_parse_params();
    if (argc - 2 != params) {
        printf("You should provide %ld param(s).\n", params);
        fputs("init: Got invalid number of params.\n", stderr);
        free(file);
        exit(127);
    }

    return 0;
}
