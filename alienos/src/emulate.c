#include "alienos.h"

int alien_emulate_end(registers *regs) {
    int status = regs->rdi;
    fprintf(stderr, "alien_end: program ended: %d\n", status);
    return 1;
}

int alien_emulate_getrand(registers *regs) {
    uint32_t rand_number;
    getrandom(&rand_number, sizeof(uint32_t), GRND_RANDOM);
    fprintf(stderr, "alien_getrand: providing random number %ld\n", rand_number);
    regs->rax = rand_number;
    return 0;
}

int alien_emulate_getkey(registers *regs) {
    char c = getchar();
    fprintf(stderr, "alien_getkey: providing key: %x\n", c);
    regs->rax = c;
    return 0;
}

int alien_emulate_print(registers *regs) {
    int x = regs->rdi;
    int y = regs->rsi;
    uint16_t chars_addr = (uint16_t)regs->rdx;
    int n = regs->r10;

    // returns the number of bytes read
    ssize_t bytes_read = 0;
    ssize_t br;

    struct iovec local_iov, remote_iov;

    while (bytes_read < n) {
        br = process_vm_readv(child,
                            const struct iovec *local_iov,
                            unsigned long liovcnt,
                            const struct iovec *remote_iov,
                            unsigned long riovcnt,
                            unsigned long flags);
        if (br == -1) {
            // -1 is returned and errno is set appropriately.
            alien_exit(127);
        }
    }

    fprintf(stderr, "alien_print: printing on (%d,%d) (%d chars) from %08x\n", x, y, n, regs->rdx);
    //alien_terminal_goto(x, y);
    //alien_terminal_show(s, n);
    return 0;
}

int alien_emulate_setcursor(registers *regs) {
    int x = regs->rdi;
    int y = regs->rsi;

    alien_terminal_goto(x, y);
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
            fprintf(stderr, "alien_emulate: invalid syscall: %d\n", regs->rax);
            return 1; // invalid syscall
    }
}

