#include "alienos.h"

void alien_exec() {
    child = fork();
    if (child == -1) {
        goto error;
    }

    if (child == 0) {
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
    waitpid(child, &status, 0);
    if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) {
        perror("invalid child state");
        goto error;
    }

    if (ptrace(PTRACE_SYSEMU, child, 0, 0) == -1) {
        goto error;
    }

    printf("child: %d\n", child);

    while(waitpid(child, &status, 0) && !WIFEXITED(status)) {
        if (ptrace(PTRACE_GETREGS, child, 0, &regs) == -1) {
            goto error;
        }

        if (alien_emulate(&regs) > 0) {
            goto error;
        }

        if (ptrace(PTRACE_SETREGS, child, 0, &regs) == -1) {
            goto error;
        }

        if (ptrace(PTRACE_SYSEMU, child, 0, 0) == -1) {
            goto error;
        }
    }

    // Code shouldn't escape while.
    // End syscall is handled at emulator side.
    perror("child unexpectedly ended");

error:
    fprintf(stderr, "alien_exec: error: %s\n", strerror(errno));
    kill(child, SIGKILL);
    exit(127);
}

