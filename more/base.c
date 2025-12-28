#include "base.h"
#include "jorkdir.h"

c full[PATH_MAX] = {0};
c real[PATH_MAX] = {0};

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
	// fuck it the previous full wipe version was on prev commits
	c**yargs=(c*[]){"yarg.bin","ystr.bin",NULL};
	size_t cnt = 0;
	while (yargs[cnt]!=NULL) {
		memset(full,0,sizeof(full));
		snprintf(full, sizeof(full), "%s/%s", dr, yargs[cnt]);
		unlink(full);
		cnt++;
	}
}