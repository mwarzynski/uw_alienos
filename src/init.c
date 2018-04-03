#include "alienos.h"

void alien_init_prepare() {
    fp = NULL;
    file = NULL;
    program_headers = NULL;
}

int alien_init_loadfile(char *filename) {
    file = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("init_loadfile: file descriptor is NULL");
        return 1;
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        perror("init_loadfile: seeking to the end of file error");
        return 1;
    }

    file_size = ftell(fp);
    if (file_size == -1) {
        perror("init_loadfile: determining file size");
        return 1;
    }

    file = malloc(sizeof(char) * (file_size + 1));
    if (file == NULL) {
        perror("init_loadfile: allocating memory to load file");
        return 1;
    }

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        perror("init_loadfile: seeking to the beginning of the file");
        return 1;
    }

    size_t len = fread(file, sizeof(char), file_size, fp);
    if (ferror(fp) != 0 ) {
        perror("init_loadfile: reading file");
        return 1;
    }

    file[len++] = '\0';

    return 0;
}

int alien_init_program_headers() {
    program_headers = malloc(sizeof(Elf64_Phdr*) * elf_header->e_phnum);
    if (program_headers == NULL) {
        perror("init_program_headers: allocating memory for program headers");
        return 1;
    }

    Elf64_Off offset = elf_header->e_phoff;
    Elf64_Half size = elf_header->e_phentsize;
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        program_headers[i] = (Elf64_Phdr*)(file + offset + i*size);
    }

    return 0;
}

int alien_init_parse_elf() {
    // ELF header - for 64-bit architecture is 64 bytes long.
    if (file_size < 64) {
        fprintf(stderr, "init_parse_elf: it's not an ELF file"
                        " (size is too small)\n");
        return 1;
    }

    elf_header = (Elf64_Ehdr*)file;

    for (size_t i = 0; i < SELFMAG; i++) {
        if (file[i] != elf_header->e_ident[i]) {
            fprintf(stderr, "init_parse_elf: magic is invalid\n");
            return 1;
        }
    }

    // ELF header - check architecture.
    if (elf_header->e_type != ELFCLASS64) {
        fprintf(stderr, "init_parse_elf: not 64-bit format\n");
        return 1;
    }

    // ELF header - check file type.
    if (elf_header->e_type != ET_EXEC) {
        fprintf(stderr, "init_parse_elf: invalid type of file\n");
        return 1;
    }

    // ELF header - check endianess.
    if ((elf_header->e_machine >> EI_DATA) != ELFDATA2LSB) {
        fprintf(stderr, "init_parse_elf: not a little-endian\n");
        return 1;
    }

    // ELF header - check architecture.
    if (elf_header->e_machine != EM_X86_64) {
        fprintf(stderr, "init_parse_elf: invalid architecture\n");
        return 1;
    }

    // ELF header - check version.
    if (elf_header->e_version != EV_CURRENT) {
        fprintf(stderr, "init_parse_elf: invalid version\n");
        return 1;
    }

    if (elf_header->e_entry < ALIEN_LOAD_ADDR_MIN
     || ALIEN_LOAD_ADDR_MAX < elf_header->e_entry) {
        fprintf(stderr, "init_parse_elf: invalid entrypoint"
                " (not in valid range)\n");
        return 1;
    }

    return 0;
}

int alien_init_params(int argc, char *argv[]) {
    // Program header of type PT_PARAMS.
    Elf64_Phdr *parameters_header = NULL;

    // Determine parameters header.
    int found = 0;
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        if (program_headers[i]->p_type == ALIEN_PT_PARAMS) {
            parameters_header = program_headers[i];
            if (found) {
                fprintf(stderr, "init_params: loading > 1 params section"
                        " is not supported\n");
                continue;
            }
            found = 1;
        }
    }

    // Evaluate number of parameters.
    Elf64_Xword paramsn = 0;
    if (parameters_header != NULL) {
        paramsn = parameters_header->p_memsz / 4;
    }

    if (argc - 2 != paramsn) {
        fprintf(stderr, "init_params: Invalid number of params,"
                " want: %d\n", paramsn);
        return 1;
    }

    // Check if address of parameter headers is valid.
    // NOTE: I assume params must be inside loaded memory via PT_LOAD header.
    found = 0;
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        if (program_headers[i]->p_type != PT_LOAD) {
            continue;
        }
        if (program_headers[i]->p_paddr > parameters_header->p_paddr) {
            continue;
        }
        Elf32_Word max_memsz = program_headers[i]->p_memsz;
        max_memsz -= (parameters_header->p_paddr - program_headers[i]->p_paddr);
        if (parameters_header->p_memsz <= max_memsz) {
            found = 1;
        }
    }
    if (!found) {
        fprintf(stderr, "init_params: there is no place to load parameters\n");
        return 1;
    }

    // Fill parameters into the block.
    int *param;
    for (Elf64_Xword i = 0; i < paramsn; i++) {
        param = (int*)(parameters_header->p_paddr + 4*i);
        *param = atoi(argv[2 + i]);
    }

    return 0;
}

int alien_init_load() {
    Elf64_Phdr *h;
    Elf64_Addr paddr, offaddr, eaddr, len;
    Elf64_Addr setaddr, setlen;
    void *mmap_ret;
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        h = program_headers[i];

        if (h->p_type != PT_LOAD) {
            continue;
        }

        if (h->p_paddr < ALIEN_LOAD_ADDR_MIN
         || ALIEN_LOAD_ADDR_MAX < h->p_paddr + h->p_memsz) {
            fprintf(stderr, "init_load: tried to allocate memory"
                    " at invalid range\n");
            return 1;
        }

        paddr = h->p_paddr & ~0xfff;
        offaddr = h->p_offset & ~0xfff;
        len = (h->p_memsz & ~0xfff) + 0x1000;
        eaddr = paddr + len;

        // Parse prot values.
        int prot = 0;
        if (h->p_flags & PF_X) {
            prot = prot | PROT_EXEC;
        }
        if (h->p_flags & PF_R) {
            prot = prot | PROT_READ;
        }
        if (h->p_flags & PF_W) {
            prot = prot | PROT_WRITE;
        }

        mmap_ret = mmap(
          (void*)paddr,                // void *addr
                len,                   // size_t len
                prot|PROT_WRITE,       // int prot
                MAP_FIXED|MAP_PRIVATE, // int flags
                fileno(fp),            // int fildes
                offaddr                // off_t off
        );
        if (mmap_ret == MAP_FAILED) {
            perror("init_load: mmap");
            return 1;
        }

        setaddr = paddr;
        setlen = h->p_paddr - paddr;
        if (setlen >= 0) {
            memset((void*)setaddr, 0, setlen);
        }

        setaddr = h->p_paddr + h->p_filesz;
        setlen = eaddr - setaddr;
        if (setlen >= 0) {
            memset((void*)setaddr, 0, setlen);
        }

        // set a valid protection memory protection
        if (mprotect((void*)paddr, len, prot) == -1) {
            perror("init_load: mprotect");
            return 1;
        }
    }

    return 0;
}

int alien_init_cleanup() {
    int ok = 0;
    if (fp != NULL) {
        if (fclose(fp) != 0) {
            ok = 1;
            perror("init_cleanup: closing file descriptor");
        }
    }
    if (program_headers != NULL) {
        free(program_headers);
    }
    if (file != NULL) {
        free(file);
    }
    return ok;
}

int alien_init(int argc, char *argv[]) {
    // Null global variables as to know later which should be freed.
    alien_init_prepare();

    // Check if there is a program to be executed.
    if (argc < 2) {
        fprintf(stderr, "init: You must specify program to execute\n");
        return 1;
    }

    // Load file content to memory.
    if (alien_init_loadfile(argv[1]) != 0) {
        return 1;
    }

    // Parse ELF header file.
    if (alien_init_parse_elf() != 0) {
        return 1;
    }

    // Parse program headers.
    if (alien_init_program_headers() != 0) {
        return 1;
    }

    // Init sections into virtual memory.
    if (alien_init_load() != 0) {
        return 1;
    }

    // Initialize parameters in virtual memory.
    if (alien_init_params(argc, argv) != 0) {
        return 1;
    }

    return 0;
}
