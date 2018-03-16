
#ifndef _INIT_H
#define _INIT_H 1

#include "alienos.h"

// Loads file into memory and accordingly parses ELF structures.
// Also, checks parameters.
// If any error occurs, alien_init will return 127.
int alien_init(int argc, char *argv[]);

#endif // _INIT_H
