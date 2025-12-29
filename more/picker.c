#include "base.h"
#include "picker.h"

void renderPicker(int wd, int hi) {
	size_t count = 0;
	memset(full,0,sizeof(full));
	c tip[] = "use UP/DOWN 2 navigate, LEFT 2 .., RIGHT to enter, ENTER to submit.";
	int reservedCols = ((wd==w)&&(w>=(sizeof(tip)+1)))*2;
	int allowedCols = hi-reservedCols;
	int off = 0;
	if (fileIdx!=0) off=(fileIdx/allowedCols)*allowedCols;
	c*truncated = malloc(wd+1);
	while (count<hi-reservedCols) {
		if (count>0) printf("\n\x1b[0m");
		if ((dirStuffSz==0)&&(count==0)) printf("\x1b[3mmt"); // i print it before any |\x1b[7m|s
		if (count+off==fileIdx) printf("\x1b[7m");
		if (count+off<dirStuffSz) {
			size_t realLen = strlen(dirStuff[count+off]);
			memset(truncated,0,wd+1);
			memcpy(truncated, dirStuff[count+off], (realLen<wd)?realLen:wd);
			if (realLen>wd) memset(truncated+wd-3,46,3);
			printf("%-*s",wd,truncated);
		}
		count++;
	}
	free(truncated);
	printf("\x1b[0m");
	if (reservedCols==0) return;
	printf("\n%s\n", tip);
	c*selected = "";
	if (dirStuffSz>0) selected = dirStuff[fileIdx];
	printf("\x1b[7m%s%s%s\x1b[0m", dir, ((*selected!=0)&&(dir[1]!=0))?"/":"", selected);
}

void renderRoot(bool isPick) {
	printf("\x1b]0;%s!\x1b\\", isPick?"yargine":"warning!!");
	c*pad;
	c*nls = calloc(h/10+1, sizeof(c)); // +1 for null term
	memset(nls, 10, h/10);
	c*name = "warning!!!";
	if (isPick) name = "y \x20  a \x20  r \x20  g \x20  i \x20  n \x20  e \x20  !";
	size_t nameLen = strlen(name);
	size_t padLen = w>nameLen ? (w-strlen(name))/2 : 0;
	pad = calloc(padLen, sizeof(c));
	memset(pad, 32, padLen);
	printf("%s%s\x1b[3%d;1m%s\x1b[0m\n%s", nls, pad, 1+(isPick*4), name, nls);
	if (!(isPick)) {
		printf("the folder you selected was detected not a yargine project (or a yargine project for an older/newer version of yargine.)\n");
		printf("please select an action using the highlighted letters, then press enter.\n\n");
		printf("\x1b[7mw\x1b[0mipe contents\n\x1b[7mc\x1b[0mancel");
		return;
	}
	printf("%s  ",nls);
	if (!(cwdValid)) printf("\x1b[1myour cwd isn't a valid yargine project!\x1b[0m ");
	printf("choose project:\n%s", nls);
	renderPicker(w,h-((h/10)*4+2));
	free(pad);
	free(nls);
}