#ifndef BASE_H
#define BASE_H
#include <stdlib.h> // might
#include <string.h> // as
#include <stdio.h> // well
#include <stdbool.h> // need
#include <termios.h> // to
#include <fcntl.h> // include
#include <unistd.h> // every
#include <signal.h> // builtin
#include <sys/ioctl.h> // lib
#include <sys/stat.h> // that
#include <limits.h> // exists...
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

#include "jorkdir.h"
#include "binaryen-c.h"
#include "readline/readline.h"
#include "readline/history.h"
#include "cwalk.h" // diabolical

typedef char c;
extern c full[PATH_MAX];
extern c real[PATH_MAX];
extern c*dir;
extern int fileIdx;
extern int dirStuffSz;
extern int w;
extern int h;
extern c**dirStuff;
extern bool cwdValid;
extern c*err;

bool isDir(c*);
void freeJorked(c**, int);
void wipeDir(c*);
#endif