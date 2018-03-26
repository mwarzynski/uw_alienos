#include "alienos.h"

void alien_terminal_goto(int x, int y) {
    printf("\033[%d;%dH", (x), (y));
}

void alien_terminal_show(char *s, int n) {
    for (int i = 0; i < n; i++) {
        fprintf(stderr, "%d\n", s[i]);
    }
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0)
            continue;
        printf("%c", s[i]);
    }
}
