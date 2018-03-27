#include "alienos.h"

void alien_exit(int code) {
    if (code == 127) {
        fprintf(stderr, "%s\n", strerror(errno));
    }

    // clean up the memory allocated while
    // initialization process
    alien_init_cleanup();

    exit(code);
}

int main(int argc, char *argv[]) {
    printf("Welcome at AlienOS!\n");

    // alien_init loads given program into memory.
    if (alien_init(argc, argv) != 0) {
        goto error;
    }

    // alien_clone makes another thread to give
    // emulated program computational power.
    alien_exec();

    alien_exit(0);
error:
    alien_exit(127);
}
