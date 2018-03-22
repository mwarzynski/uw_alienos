#include "alienos.h"

void alien_terminal_goto(int x, int y) {
    printf("\033[%d;%dH", (x), (y));
}

void alien_terminal_show(char *s, int n) {
    printf("%.*s", n, s);
}
