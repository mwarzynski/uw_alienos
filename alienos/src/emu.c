#include <stdio.h>

#include <alienos.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 127;
    }

    printf("Welcome at AlienOS!\n");
    alien_init(argc, argv);

    return 0;
}
