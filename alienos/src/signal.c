#include "alienos.h"

void handle_signal(int signal) {
    switch (signal) {
        case SIGINT:
            perror("signal_handler: received SIGINT");
            alien_exit(127);
        default:
            perror("signal_handler: caught wrong signal");
            alien_exit(127);
    }
}

void alien_setup_signal_handler() {
    struct sigaction sa;

    sa.sa_handler = &handle_signal;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("setup_signal: cannot handle SIGINT");
        alien_exit(127);
    }
}
