#include "base.h"
#include "picker.h"

void renderPicker(int wd, int hi) {
	size_t count = 0;
	memset(full,0,sizeof(full));
	c tip[] = "use UP/DOWN 2 navigate, LEFT 2 .., RIGHT to enter dir, ENTER to submit.";
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
	printf("%s  \x1b[1myour cwd isn't a valid yargine project!\x1b[0m choose project:\n%s",nls,nls);
	renderPicker(w,h-((h/10)*4+2));
	free(pad);
	free(nls);
}

int x,y,wd,xOff;

void __PRINT(c*s, ...) {
	va_list args;
	va_list args1;
	va_start(args, s);
	va_copy(args1, args);
	size_t needed = vsnprintf(NULL, 0, s, args1);
	va_end(args1);
	c*buf = calloc(needed+1,sizeof(c)); // auto-memsets!
	vsprintf(buf, s, args);
	va_end(args);
	size_t cnt = 0;
	bool isNl, isLead;
	while (cnt<needed) {
		isNl = ((buf[cnt]==10)||(buf[cnt]==13));
		if (((xOff%wd==0)&&(xOff>0))||(isNl)) {
			xOff=0;
			printf("\n\x1b[%dG", x+1);
			if (isNl) {
				if ((buf[cnt]==13)&&(buf[cnt+1]==10)) cnt++;
				cnt++;
				continue;
			}
		}
		while ((buf[cnt]==32)&&(xOff==0)) cnt++;
		putc(buf[cnt],stdout);
		xOff++;
		cnt++;
	}
	free(buf);
}

void renderPROJ_View(int _x, int _y, int _wd, int hi) {
	x=_x;
	y=_y;
	wd=_wd;
	xOff=0;
	ansiGoTo(x,y); // setup pos
	size_t vpsLen = strlen(vp2Str[pvMode]);
	c*philler = calloc(wd-vpsLen-3,sizeof(c));
	memset(philler,58,wd-vpsLen-4);
	__PRINT(":: ");
	printf("\x1b[3m%s\x1b[0m",vp2Str[pvMode]);
	__PRINT(" %s\n",philler); // resets the xOff so future |__PRINT|s dont attempt to print on line №1
	switch (pvMode) {
		case INTRO:
			__PRINT("welcome to ");
			printf("\x1b[35;1m");
			__PRINT("yargine");
			printf("\x1b[0m!\n");
			__PRINT("\ncheck out the ");
			printf("\x1b[1m");
			__PRINT("github repo");
			printf("\x1b[0m");
			__PRINT(
				": https://github.com/RixInGithub/yargine/\n"
				"\n"
				"yargine is a game engine made in pure c for the newgens that has an editor on the terminal! "
				"(since they'll have NO IDEA what a \"terminal\" is)\n"
				"\n"
				"press P to scroll up this text, and L to scroll down." // wip
			);
			break;
		case SETT:
			__PRINT("wip\n");
			break;
		case EXPT:
			__PRINT("plz pick an export from these presets i made myself:\n\nwip");
			break;
		case YRGS:
			__PRINT("wip\n");
			break;
	}
}