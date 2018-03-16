
#ifndef _INIT_H
#define _INIT_H 1

#include "alienos.h"

// Load file and accordingly parse ELF structures.
// Also, check parameters.
// If any error occurs, alien_init will return 127.
int alien_init(int argc, char *argv[]);

#endif // _INIT_H
