#ifndef LYRG_H
#define LYRG_H
#include "base.h"
typedef struct LyrgCmd {
	c type;
	c*cmd;
	void*extraData;
} LyrgCmd;

typedef struct LyrgRes {
	c*err;
	LyrgCmd*chain;
} LyrgRes;

LyrgRes*parseLyrg(c*);
#endif