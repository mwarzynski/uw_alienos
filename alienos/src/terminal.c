#include "alienos.h"

void alien_terminal_init() {
    struct termios t;
	tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= (~ICANON & ~ECHO);
	tcsetattr(STDIN_FILENO,TCSANOW, &t);

    alien_terminal_clear();
    terminal_x = 0;
    terminal_y = 0;
    alien_terminal_goto(terminal_x, terminal_y);

    fprintf(stderr, "sizeof alien_char: %d\n", sizeof(alien_char));
}

void alien_terminal_clear() {
    printf("\033[2J");
}

void alien_terminal_goto(int x, int y) {
    printf("\033[%d;%df", y+1, x+1);
}

void alien_terminal_show(alien_char *s, int n) {
    for (int i = 0; i < n; i++) {
        fprintf(stderr, "termina_show(%d,%d): '%c'\n", terminal_x+i, terminal_y, s[i].c);
        printf("%c", s[i].c);
    }
}
