
#ifndef _SYSCALL_H
#define _SYSCALL_H 1

#include "alienos.h"

// Syscall hints:
// The number of the system call is passed in the register rax.
// The return value from the call is also in the register rax,

#define NORETURN 0
// void noreturn end(int status)
//                   rdi

#define GETRAND 1
// uint32_t getrand()
//

#define GETKEY 2
// int getkey()
//

#define PRINT 3
// void print(int x, int y, uint16_t *chars, int n)
//            rdi  , rsi  , rdx            , r10

#define SETCURSOR 4
// void setcursor(int x, int y)
//                rdi  , rsi

#endif // _SYSCALL_H
