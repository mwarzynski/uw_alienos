#include "alienos.h"


int alien_init_loadfile(char *filename) {
    file = NULL;

    fp = fopen(filename, "rwx");
    if (fp == NULL) {
        perror("init_loadfile: File descriptor is NULL.");
        return 1;
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        perror("init_loadfile: Seeking to the end of file error.");
        return 1;
    }

    file_size = ftell(fp);
    if (file_size == -1) {
        perror("init_loadfile: Could not determine file size.");
        return 1;
    }

    file = malloc(sizeof(char) * (file_size + 1));

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        perror("init_loadfile: Could not seek to the file beggining.");
        return 1;
    }

    size_t len = fread(file, sizeof(char), file_size, fp);
    if (ferror(fp) != 0 ) {
        perror("init_loadfile: Error reading file.");
        return 1;
    }

    file[len++] = '\0';

    return 0;
}

int alien_init_program_headers() {
    program_headers = malloc(sizeof(Elf64_Phdr*) * elf_header->e_phnum);
    if (program_headers == NULL) {
        // malloc failed to allocate memory
        perror("could not allocate memory");
        return 1;
    }

    Elf64_Off offset = elf_header->e_phoff;
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        program_headers[i] = (Elf64_Phdr*)(file + offset + i*(elf_header->e_phentsize));
    }

    return 0;
}

int alien_init_section_headers() {
    section_headers = malloc(sizeof(Elf64_Shdr*) * elf_header->e_shnum);
    if (section_headers == NULL) {
        // malloc failed to allocate memory
        perror("could not allocate memory");
        return 1;
    }

    Elf64_Off offset = elf_header->e_shoff;
    for (size_t i = 0; i < elf_header->e_shnum; i++) {
        section_headers[i] = (Elf64_Shdr*)(file + offset + i*(elf_header->e_shentsize));
    }

    return 0;
}

int alien_init_parse_elf() {
    // ELF header - for 64-bit architecture is 64 bytes long.
    if (file_size < 64) {
        perror("It's not an ELF file.");
        return 1;
    }

    elf_header = (Elf64_Ehdr*)file;

    for (size_t i = 0; i < SELFMAG; i++) {
        if (file[i] != elf_header->e_ident[i]) {
            perror("ELF header: invalid magic number.");
            return 1;
        }
    }

    // ELF header - check architecture.
    if (elf_header->e_type != ELFCLASS64) {
        perror("ELF header - not 64-bit format.");
        return 1;
    }

    // ELF header - check file type.
    if (elf_header->e_type != ET_EXEC) {
        perror("ELF header - invalid type of file");
        return 1;
    }

    // ELF header - check endianess.
    if ((elf_header->e_machine >> EI_DATA) != ELFDATA2LSB) {
        perror("ELF header - not a little-endian");
        return 1;
    }

    // ELF header - check architecture.
    if (elf_header->e_machine != EM_X86_64) {
        perror("ELF header -invalid architecture");
        return 1;
    }

    // ELF header - check version.
    if (elf_header->e_version != EV_CURRENT) {
        perror("ELF header - invalid version");
        return 1;
    }

    return 0;
}

int alien_init_params(int argc, char *argv[]) {
    parameters_header = NULL;

    // Determine parameters header.
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        if (program_headers[i]->p_type == ALIEN_PT_PARAMS) {
            parameters_header = program_headers[i];
            break;
        }
    }

    // Evaluate number of parameters.
    Elf64_Xword paramsn = 0;
    if (parameters_header != NULL) {
        paramsn = parameters_header->p_memsz / 4;
    }

    if (argc - 2 != paramsn) {
        perror("Invalid number of params");
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
            // errno is set by mmap
            return 1;
        }

        // memset 0
        setaddr = paddr;
        setlen = h->p_paddr - paddr;

        if (setlen) {
            memset((void*)setaddr, 0, setlen);
        }

        setaddr = h->p_paddr + h->p_filesz;
        setlen = eaddr - setaddr;
        if (setlen) {
            memset((void*)setaddr, 0, setlen);
        }
    }

    return 0;
}

void alien_init_cleanup() {
    // TODO: implement cleaning up after initialization
    free(file);
}

int alien_init(int argc, char *argv[]) {
    if (argc < 2) {
        perror("You must specify program to execute");
        goto error;
    }

    // Load file content to memory.
    if (alien_init_loadfile(argv[1]) != 0) {
        goto error;
    }

    // Parse ELF header file.
    if (alien_init_parse_elf() != 0) {
        goto error;
    }

    // Parse program headers.
    if (alien_init_program_headers() != 0) {
        goto error;
    }

    // ELF header - parse section header entries.
    if (alien_init_section_headers() != 0) {
        goto error;
    }

    // Init sections into virtual memory.
    if (alien_init_load() != 0) {
        goto error;
    }

    // Initialize parameters in virtual memory.
    if (alien_init_params(argc, argv) != 0) {
        goto error;
    }

    return 0;
error:
    alien_init_cleanup();
    return 1;
    fprintf(stderr, "alien_init: %s\n", strerror(errno));
    exit(127);
}
