#include <stdio.h>

#include <alienos.h>
#include <emu.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 127;
    }

    printf("Welcome at AlienOS!\n");

    return 0;
}
