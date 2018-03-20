#include "alienos.h"

int main(int argc, char *argv[]) {
    printf("Welcome at AlienOS!\n");

    // alien_init loads given program into memory.
    alien_init(argc, argv);

    // alien_clone makes another thread to give
    // emulated program computational power.
    alien_exec();

    return 0;
}
