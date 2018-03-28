#include "alienos.h"

int alien_terminal_init() {
    struct termios t;
	tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= (~ICANON & ~ECHO);
	tcsetattr(STDIN_FILENO,TCSANOW, &t);

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    if (w.ws_col < ALIEN_TERMINAL_WIDTH) {
        fprintf(stderr, "terminal_init: not sufficient terminal width\n");
        return 1;
    }
    if (w.ws_row < ALIEN_TERMINAL_HEIGHT) {
        fprintf(stderr, "terminal_init: not sufficient terminal height\n");
        return 1;
    }

    alien_terminal_clear();
    terminal_x = 0;
    terminal_y = 0;
    alien_terminal_goto(terminal_x, terminal_y);

    return 0;
}

int alien_terminal_color(uint8_t c) {
    c &= ~(1UL << 7);
    c &= ~(1UL << 8);

    switch (c) {
        case ALIEN_COLOR_BLACK:
            return 30;
        case ALIEN_COLOR_BLUE:
            return 34;
        case ALIEN_COLOR_GREEN:
            return 32;
        case ALIEN_COLOR_TURQUOISE:
            return 36;
        case ALIEN_COLOR_RED:
            return 31;
        case ALIEN_COLOR_PINK:
            return 35;
        case ALIEN_COLOR_YELLOW:
            return 33;
        case ALIEN_COLOR_GREY_LIGHT:
            return 37;
        case ALIEN_COLOR_GREY_DARK:
            return 90;
        case ALIEN_COLOR_BRIGHT_BLUE:
            return 94;
        case ALIEN_COLOR_BRIGHT_GREEN:
            return 92;
        case ALIEN_COLOR_BRIGHT_TURQUOISE:
            return 96;
        case ALIEN_COLOR_BRIGHT_RED:
            return 91;
        case ALIEN_COLOR_BRIGHT_PINK:
            return 95;
        case ALIEN_COLOR_BRIGHT_YELLOW:
            return 93;
        case ALIEN_COLOR_WHITE:
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
