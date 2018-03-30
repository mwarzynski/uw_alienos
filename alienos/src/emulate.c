#include "alienos.h"

int alien_emulate_end(registers *regs) {
    int status = regs->rdi;

    if (status < ALIEN_END_CODE_MIN || ALIEN_END_CODE_MAX < status) {
        fprintf(stderr, "emulate_end: invalid end code %d\n");
        alien_exit(127);
    }
    fprintf(stderr, "emulate_end: program ended: %d\n", status);

    alien_exit(status);
}

int alien_emulate_getrand(registers *regs) {
    if (getrandom(&regs->rax, sizeof(uint32_t), 0) == -1) {
        perror("emulate_getrand: getrandom");
        return 1;
    }
    return 0;
}

int alien_emulate_key_invalid(int c) {
    if (c != ALIEN_KEY_ENTER
     && c != ALIEN_KEY_UP
     && c != ALIEN_KEY_DOWN
     && c != ALIEN_KEY_LEFT
     && c != ALIEN_KEY_RIGHT
     && (c < ALIEN_KEY_ASCII_MIN || ALIEN_KEY_ASCII_MAX < c)) {
        return 1;
    }
    return 0;
}

int alien_emulate_key_arrow() {
    int c;
    if ((c = getchar()) != 0x5b) {
        return 0;
    }
    c = getchar();
    switch (c) {
        case 0x44:
            c = ALIEN_KEY_LEFT;
            break;
        case 0x43:
            c = ALIEN_KEY_RIGHT;
            break;
        case 0x42:
            c = ALIEN_KEY_DOWN;
            break;
        case 0x41:
            c = ALIEN_KEY_UP;
            break;
        default:
            c = 0x0;
    }
    return c;
}

int alien_emulate_getkey(registers *regs) {
    int c = 0x0;

    while ((c = getchar()) != EOF) {
        if (c == 0x1b) {
            c = alien_emulate_key_arrow();
        }
        if (alien_emulate_key_invalid(c)) {
            continue;
        }
        break;
    }
    if (c == EOF) {
        fprintf(stderr, "emulate_getkey: got EOF, providing 0x0\n");
        c = 0x0;
    }

    regs->rax = c;
    return 0;
}

int alien_emulate_print(registers *regs) {
    int x = regs->rdi;
    int y = regs->rsi;
    int n = regs->r10;

    if (n < 0 || 80 < n) {
        fprintf(stderr, "emulate_print: invalid string length.\n");
        return 1;
    }

    size_t buffer_size = sizeof(alien_char) * n;
    alien_char *buffer = malloc(buffer_size);

    struct iovec local_iov, remote_iov;
    local_iov.iov_base = buffer;
    local_iov.iov_len = buffer_size;
    remote_iov.iov_base = (void*)regs->rdx;
    remote_iov.iov_len = buffer_size;

    ssize_t br = process_vm_readv(alien_child,
                        &local_iov,
                        1,
                        &remote_iov,
                        1,
                        0);
    if (br != buffer_size) {
        free(buffer);
        perror("emulate_print: reading child memory (process_vm_readv)");
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
    return 0;
}

int alien_emulate_syscall(registers *regs) {
    switch (regs->orig_rax) {
        case ALIEN_SYSCALL_END:
            return alien_emulate_end(regs);
        case ALIEN_SYSCALL_GETRAND:
            return alien_emulate_getrand(regs);
        case ALIEN_SYSCALL_GETKEY:
            return alien_emulate_getkey(regs);
        case ALIEN_SYSCALL_PRINT:
            return alien_emulate_print(regs);
        case ALIEN_SYSCALL_SETCURSOR:
            return alien_emulate_setcursor(regs);
        default:
            fprintf(stderr, "emulate_syscall: invalid syscall %x\n", regs->orig_rax);
            return 0;
    }
}

int alien_emulate() {
    if (alien_terminal_init() != 0) {
        goto error;
    }

    if (ptrace(PTRACE_SYSEMU, alien_child, 0, 0) == -1) {
        perror("emulate: ptrace sysemu");
        return 1;
    }

    registers regs;
    int status;
    while(waitpid(alien_child, &status, 0) && !WIFEXITED(status)) {
        if (ptrace(PTRACE_GETREGS, alien_child, 0, &regs) == -1) {
            perror("emulate: ptrace getregs");
            goto error;
        }

        if (alien_emulate_syscall(&regs) != 0) {
            goto error;
        }

        if (ptrace(PTRACE_SETREGS, alien_child, 0, &regs) == -1) {
            perror("emulate: ptrace setregs");
            goto error;
        }

        if (ptrace(PTRACE_SYSEMU, alien_child, 0, 0) == -1) {
            perror("emulate: ptrace sysemu");
            goto error;
        }
    }

    // Code shouldn't escape while.
    // The correct way to end the program is syscall end.
    fprintf(stderr, "emulate: child unexpectedly died\n");

error:
    alien_exit(127);
}

