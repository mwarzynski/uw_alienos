#include "alienos.h"

int alien_exec() {
    alien_child = fork();
    if (alien_child == -1) {
        perror("exec: fork");
        return 1;
    }

    if (alien_child == 0) {
		// Child executes given program code.

        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
            perror("CHILD::exec: ptrace traceme");
            exit(1);
        }

        // Wait for the parent to start tracing.
        if (kill(getpid(), SIGSTOP) == 1) {
            fprintf(stderr, "CHILD::exec: kill(myself, SIGSTOP) error\n");
            exit(1);
        }

        // Jump to the entrypoint.
        void (*func)() = (void (*)())elf_header->e_entry;
        func();

        exit(0);
    }

    // Get kill syscall with SIGSTOP.
    int status;
    waitpid(alien_child, &status, 0);
    if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) {
        fprintf(stderr, "exec: invalid alien_child state\n");
        return 1;
    }

    // Send a SIGKILL signal to the tracee if the tracer exits.
    if (ptrace(PTRACE_SETOPTIONS, alien_child, NULL, PTRACE_O_EXITKILL) == -1) {
        perror("exec: ptrace setoptions exitkill");
        return 1;
    }

    return 0;
}

