#include "alienos.h"

void alien_exec() {
    child = fork();

    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        asm("jmp %0" : "=" (elf_header->e_entry));
    }

    if (ptrace(PTRACE_SYSEMU, child, 0, 0) == -1) {
        fprintf(stderr, "alien_exec: ptrace_sysemu err: %s\n", strerror(errno));
        exit(127);
    }

    struct user_regs_struct regs;
    int status;

    while(waitpid(child, &status, 0) && ! WIFEXITED(status)) {
        ptrace(PTRACE_GETREGS, child, NULL, &regs);

        printf("REG: %d\n", regs.orig_rax);

        if (ptrace(PTRACE_SYSEMU, child, 0, 0) == -1) {
            fprintf(stderr, "alien_exec: ptrace_sysemu err: %s\n", strerror(errno));
            exit(127);
        }
    }
}

