#include "alienos.h"

int alien_emulate_end(registers *regs) {
    int status = regs->rdi;
    fprintf(stderr, "alien_end: program ended: %d\n", status);
    return 1;
}

int alien_emulate_getrand(registers *regs) {
    if (getrandom(&regs->rax, sizeof(uint32_t), GRND_RANDOM) == -1) {
        return 1;
    }
    return 0;
}

int alien_emulate_key_invalid(char c) {
    fprintf(stderr, "alien_key_isvalid: '%c' (%x)\n", c, c);
    if (c != KEY_ENTER
     && c != KEY_UP
     && c != KEY_DOWN
     && c != KEY_LEFT
     && c != KEY_RIGHT
     && (c < 0x20 || 0x7e < c)) {
        return 1;
    }
    return 0;
}

int alien_emulate_getkey(registers *regs) {
    char c = 0x0;

    while ((c = getchar()) != EOF) {
        if (alien_emulate_key_invalid(c)) {
            continue;
        }
        break;
    }

    fprintf(stderr, "alien_getkey: providing key: '%c' (%x)\n", c, c);
    regs->rax = c;
    return 0;
}

int alien_emulate_print(registers *regs) {
    int x = regs->rdi;
    int y = regs->rsi;
    int n = regs->r10;

    if (n < 0 || 80 < n) {
        perror("print: invalid string length");
        return 1;
    }

    size_t buffer_size = sizeof(alien_char) * n;
    alien_char *buffer = malloc(buffer_size);

    struct iovec local_iov, remote_iov;
    local_iov.iov_base = buffer;
    local_iov.iov_len = buffer_size;
    remote_iov.iov_base = (void*)regs->rdx;
    remote_iov.iov_len = buffer_size;

    ssize_t br = process_vm_readv(child,
                        &local_iov,
                        1,
                        &remote_iov,
                        1,
                        0);
    if (br != buffer_size) {
        return 1;
    }

    alien_terminal_goto(x, y);
    alien_terminal_show(buffer, n);
    alien_terminal_goto(terminal_x, terminal_y);

    free(buffer);
    return 0;
}

int alien_emulate_setcursor(registers *regs) {
    terminal_x = regs->rdi;
    terminal_y = regs->rsi;

    alien_terminal_goto(terminal_x, terminal_y);
    fprintf(stderr, "alien_setcursor = (%d, %d)\n", terminal_x, terminal_y);
    return 0;
}

int alien_emulate(registers *regs) {
    switch (regs->orig_rax) {
        case SYSCALL_END:
            return alien_emulate_end(regs);
        case SYSCALL_GETRAND:
            return alien_emulate_getrand(regs);
        case SYSCALL_GETKEY:
            return alien_emulate_getkey(regs);
        case SYSCALL_PRINT:
            return alien_emulate_print(regs);
        case SYSCALL_SETCURSOR:
            return alien_emulate_setcursor(regs);
        default:
            perror("alien_emulate: invalid syscall");
            return 1;
    }
}

