#include "alienos.h"


void alien_init_loadfile(char *filename) {
    file = NULL;

    fp = fopen(filename, "rwx");
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

void alien_init_params(int argc, char *argv[]) {
    parameters_header = NULL;

    // Determine parameters header.
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        if (program_headers[i]->p_type == PT_PARAMS) {
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
        fprintf(stderr, "init_params: invalid number of parameters (want: %d).\n", paramsn);
        exit(127);
    }

    // Fill parameters into the block.
    int *param;
    for (Elf64_Xword i = 0; i < paramsn; i++) {
        param = (int*)(parameters_header->p_paddr + 4*i);
        *param = atoi(argv[2 + i]);
    }
}

void alien_init_load() {
    Elf64_Phdr *h;
    Elf64_Addr paddr, offaddr, len;
    void *mmap_ret;
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        h = program_headers[i];

        if (h->p_type != PT_LOAD) {
            continue;
        }

        paddr = h->p_paddr & ~0xfff;
        offaddr = h->p_offset & ~0xfff;
        len = (h->p_memsz & ~0xfff) + 0x1000;

        printf("%08x\n", h->p_flags);

        mmap_ret = mmap(
          (void*)paddr,                // void *addr
                len,                   // size_t len
                h->p_flags,            // int prot
                MAP_FIXED|MAP_PRIVATE, // int flags
                fileno(fp),            // int fildes
                offaddr                // off_t off
        );

        if (mmap_ret == MAP_FAILED) {
            fprintf(stderr, "init_load: mmap: %s\n", strerror(errno));
        }
    }
}

void alien_init_memory_prepare() {
    // TODO: implement setting unused memory to zero
    // It's due to mapping with alignment which causes
    // loading more bytes from file than needed.
    // Especially, overwriting BSS section might not be
    // something we want.
}

void alien_init_cleanup() {
    // TODO: implement cleaning up after initialization
}

void alien_init(int argc, char *argv[]) {
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

    // Init sections into virtual memory.
    alien_init_load();

    // Initialize parameters in virtual memory.
    alien_init_params(argc, argv);

    // Prepare the memory.
    // Well, memset zero.
    alien_init_memory_prepare();
}
