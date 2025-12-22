#ifndef BASE_H
#define BASE_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

typedef char c;
extern c full[PATH_MAX];
extern c real[PATH_MAX];
extern c*dir;
#endif
