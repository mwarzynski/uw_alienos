#include "alienos.h"

void alien_exit(int code) {
    if (code == 127) {
        fprintf(stderr, "%s\n", strerror(errno));
    }

    // clean up the memory allocated while
    // initialization process
    alien_init_cleanup();

    // kill the child
    if (child != 0) {
        kill(SIGKILL, child);
    }

    exit(code);
}

int main(int argc, char *argv[]) {
    printf("Welcome at AlienOS!\n");

    // alien_init loads given program into memory.
    alien_init(argc, argv);

    // alien_clone makes another thread to give
    // emulated program computational power.
    alien_exec();

    // alien_exit cleans up memory and handles
    // killing the child process (if exists)
    alien_exit(0);
}
