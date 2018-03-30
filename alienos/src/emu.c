#include "alienos.h"

void alien_exit(int code) {
    // clean up the memory allocated while
    // initialization process
    if (alien_init_cleanup() != 0) {
        exit(127);
    }

    exit(code);
}

int main(int argc, char *argv[]) {
    // alien_init loads given program into memory.
    if (alien_init(argc, argv) != 0) {
        goto error;
    }

    // alien_exec executes given program
    // the alien operating system.
    // Exec should call the alien_exit directly
    // when syscall end is received.
    if (alien_exec() != 0) {
        goto error;
    }

    alien_emulate();

error:
    alien_exit(127);
}
