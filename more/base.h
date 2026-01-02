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
#include <stdarg.h>

#include "jorkdir.h"
#include "binaryen-c.h"
#include "readline/readline.h"
#include "readline/history.h"
#include "cwalk.h"

#ifdef __YONLINE
	#define TEXTRA " online"
#else
	#define TEXTRA ""
#endif

typedef enum {
	INTRO,
	SETT,
	EXPT,
	YRGS,
	PVIEWS
} PViewMode;
typedef char c;
extern c full[PATH_MAX];
extern c real[PATH_MAX];
extern c*dir;
extern c*projDir;
extern int fileIdx;
extern int dirStuffSz;
extern int w;
extern int h;
extern c**dirStuff;
extern c*err;
extern PViewMode pvMode;
extern c**vp2Str;
extern c**vp2Ch;
extern c*expts[];

bool isDir(c*);
void freeJorked(c**, int);
void wipeDir(c*);
void openFileWithGUI(c*);
void ansiGoTo(int, int);
void initPvMode();
#endif