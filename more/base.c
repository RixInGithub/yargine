#include "base.h"
#include "jorkdir.h"

c full[PATH_MAX] = {0};
c real[PATH_MAX] = {0};
c*dir;

bool isDir(c*p) {
	struct stat st;
	return ((lstat(p, &st) == 0) && (S_ISDIR(st.st_mode)));
}

void freeJorked(c**jorked, int sz) {
	int count = 0;
	while (count<sz) free(jorked[count++]); // increments count, gets jorked[prevCount] and frees it. simple!
	free(jorked);
}

void wipeDir(c*dr) {
	int jorkedSz;
	c**jorked = jorkdir(dr,&jorkedSz);
	memset(full,0,sizeof(full));
	int count = 0;
	while (count<jorkedSz) {
		c*unfinished = jorked[count];
		if (strcmp(unfinished,"..")) {
			snprintf(full, sizeof(full), "%s/%s", dr, unfinished);
			switch ((int)isDir(full)) {
				case 1:
					wipeDir(full);
					rmdir(full);
					break;
				case 0:
					unlink(full);
					break;
				default:
					__builtin_unreachable();
					break;
			}
		}
		count++;
	}
	freeJorked(jorked, jorkedSz);
}