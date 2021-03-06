#include "alienos.h"

struct termios *terminal_termios = NULL;

void alien_terminal_clear() {
    printf("\033[2J");
}

int alien_terminal_getsize(struct winsize *w) {
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, w) == -1) {
        perror("terminal_init: getting terminal size (ioctl)");
        return 1;
    }
    return 0;
}

int alien_terminal_init() {
    terminal_termios = malloc(sizeof(struct termios));
    if (terminal_termios == NULL) {
        perror("terminal_init: malloc");
        return 1;
    }

    tcgetattr(STDIN_FILENO, terminal_termios);

    struct termios t = *terminal_termios;
    t.c_lflag &= (~ICANON & ~ECHO);
	if (tcsetattr(STDIN_FILENO,TCSANOW, &t) == -1) {
        perror("terminal_init: setting terminal flags (tcsetattr)");
        return 1;
    }

    alien_terminal_clear();

    terminal_x = 0;
    terminal_y = 0;
    alien_terminal_goto(terminal_x, terminal_y);

    struct winsize w;
    if (alien_terminal_getsize(&w) == 0) {
        if (w.ws_col < ALIEN_TERMINAL_WIDTH_MIN
         || w.ws_row < ALIEN_TERMINAL_HEIGHT_MIN) {
            fprintf(stderr, "terminal_init: your terminal is too small,"
                            " aliens use 80x24 terminals\n");
        }
    }

    return 0;
}

void alien_terminal_cleanup() {
    if (terminal_termios == NULL) {
        return;
    }
	if (tcsetattr(STDIN_FILENO,TCSANOW, terminal_termios) == -1) {
        perror("terminal_cleanup: setting previous terminal flags");
        return;
    }
}

int alien_terminal_color(uint8_t c) {
    c &= 0x0f;
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
            fprintf(stderr, "terminal_color: got invalid color %d\n", c);
            return 39;
    }
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
