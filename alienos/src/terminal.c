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

int alien_terminal_color(uint8_t c) {
    c &= ~(1UL << 7);
    c &= ~(1UL << 8);

    switch (c) {
        case COLOR_BLACK:
            return 30;
        case COLOR_BLUE:
            return 34;
        case COLOR_GREEN:
            return 32;
        case COLOR_TURQUOISE:
            return 36;
        case COLOR_RED:
            return 31;
        case COLOR_PINK:
            return 35;
        case COLOR_YELLOW:
            return 33;
        case COLOR_GREY_LIGHT:
            return 37;
        case COLOR_GREY_DARK:
            return 90;
        case COLOR_BRIGHT_BLUE:
            return 94;
        case COLOR_BRIGHT_GREEN:
            return 92;
        case COLOR_BRIGHT_TURQUOISE:
            return 96;
        case COLOR_BRIGHT_RED:
            return 91;
        case COLOR_BRIGHT_PINK:
            return 95;
        case COLOR_BRIGHT_YELLOW:
            return 93;
        case COLOR_WHITE:
            return 37;
        default:
            return 39;
    }
}

void alien_terminal_clear() {
    printf("\033[2J");
}

void alien_terminal_goto(int x, int y) {
    printf("\033[%d;%df", y+1, x+1);
}

void alien_terminal_show(alien_char *s, int n) {
    int color;
    for (int i = 0; i < n; i++) {
        color = alien_terminal_color(s[i].color);
        printf("\033[%dm%c\033[39m", color, s[i].c);
    }
}
