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
    int n = regs->r10 * 2;

    if (n < 0 || 1024*1024 < n) {
        perror("print: invalid string length");
        return 1;
    }

    char *buffer = malloc(sizeof(char) * n);

    // returns the number of bytes read
    ssize_t bytes_read = 0;
    ssize_t br;

    struct iovec local_iov, remote_iov;


    for (int i = 0; i < n; i++) {
        buffer[i] = 'A';
    }

    local_iov.iov_base = buffer;
    local_iov.iov_len = n;

    remote_iov.iov_base = (void*)regs->rdx;
    remote_iov.iov_len = n;

    br = process_vm_readv(child,
                        &local_iov,
                        1,
                        &remote_iov,
                        1,
                        0);
    if (br != n) {
        fprintf(stderr, "process vm readv: %s\n", strerror(errno));
        return 1;
    }

    alien_terminal_goto(x, y);
    alien_terminal_show(buffer, n);

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
            perror("alien_emulate: invalid syscall");
            return 1;
    }
}

