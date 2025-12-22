#ifndef YSTR_H
#define YSTR_H
#include "base.h"
#define YSMAJOR 0 // this should only be changed when the structure's data types are swapped around.

typedef uint32_t ystrIdx; // 1-based. 0 = blank
typedef c yHdr[4];
#pragma pack(push,1)
typedef struct {
	yHdr hdr;
	uint8_t majorVer;
	ystrIdx projName;
	ystrIdx main;
} yarg;
#pragma pack(pop)
extern yarg thisProj;

bool initYarg(c*,c*);
ystrIdx genYstr(c*);
bool modifyYstr(ystrIdx, c*);
bool readYarg();
c* readYstr(ystrIdx);
#endif
