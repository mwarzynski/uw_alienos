#include "alienos.h"

void alien_exec() {
    alien_child = fork();
    if (alien_child == -1) {
        goto error;
    }

    if (alien_child == 0) {
		// Child executes given program code.

        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        kill(getpid(), SIGSTOP); // Wait for the parent to start tracing.

        // Jump to the entrypoint.
        void (*func)() = (void (*)())elf_header->e_entry;
        func();

        exit(0);
    }

	// Parent emulates AlienOS using PTRACE.
    registers regs;
    int status;

    // Get kill syscall with SIGSTOP.
    waitpid(alien_child, &status, 0);
    if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) {
        perror("invalid alien_child state");
        goto error;
    }

    // Send a SIGKILL signal to the tracee if the tracer exits.
    ptrace(PTRACE_SETOPTIONS, alien_child, NULL, PTRACE_O_EXITKILL);

    if (ptrace(PTRACE_SYSEMU, alien_child, 0, 0) == -1) {
        goto error;
    }

    if (alien_terminal_init() != 0) {
        goto error;
    }

    while(waitpid(alien_child, &status, 0) && !WIFEXITED(status)) {
        if (ptrace(PTRACE_GETREGS, alien_child, 0, &regs) == -1) {
            goto error;
        }

        if (alien_emulate(&regs) > 0) {
            goto error;
        }

        if (ptrace(PTRACE_SETREGS, alien_child, 0, &regs) == -1) {
            goto error;
        }

        if (ptrace(PTRACE_SYSEMU, alien_child, 0, 0) == -1) {
            goto error;
        }
    }

    // Code shouldn't escape while.
    // End syscall is handled at emulate.c side.
    fprintf(stderr, "child unexpectedly ended\n");

error:
    alien_exit(127);
}

