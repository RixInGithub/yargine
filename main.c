#include <stdlib.h> // AHHHH IT FEELS SO GOOD TO WRITE <stdlib.h> WOOHOO
#include <string.h> // we're so back!
#include <stdio.h> // ohhh yes
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
#include "jorkdir/jorkdir.h"
#include "binaryen-c.h"
#define _getch getchar

typedef enum {
	PICK,
	NONYARGWARN,
	PROJ
} RenderMode;

typedef char c;
volatile sig_atomic_t running = 1, resized = 1;
int w, h, fileIdx, dirStuffSz;
bool needsRender;
struct termios oldt;
c*err = "";
c*dir;
char**dirStuff;
c**onlyDirs;
int onlyDirsSz = 0;
RenderMode renderM;

// strings file => "ystr.bin"
// base project => "yarg.bin"

typedef struct {
	c hdr[4];
	int projNameIdx; // usually 0 idk
} yarg;

yarg*readYarg() {

}

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
	c full[PATH_MAX] = {0};
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

void initYarg(c*name) {
	;
}

void getTermSize() {
	if (!(resized)) return;
	resized = 0;
	struct winsize ws;
	if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws)==-1){w=0;h=0;return;}
	w=ws.ws_col;
	h=ws.ws_row;
}

void winch(int _) {
	resized = 1;
}

void exitHand(int _) {
	running=0;
}

bool _kbhit() {
	int ch = getchar();
	bool res = ch!=EOF;
	if (res) ungetc(ch, stdin);
	return res;
}

void switchBufs(int b) {
	switch (b) {
		case 1:
			printf("\x1b[?25h\x1b[2J\x1b[H\x1b[?1049l");
			break;
		case 2:
			printf("\x1b[?1049h\x1b[?25l\x1b[2J\x1b[H");
			break;
		default: break;
	}
}

__attribute__((destructor)) void cleanup() {
	printf("\x1b[?1003l\x1b[?1006l");
	fflush(stdout);
	switchBufs(1);
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	free(dir);
	freeJorked(dirStuff, dirStuffSz);
	freeJorked(onlyDirs, onlyDirsSz);
	if (*err==0) return;
	printf("\x1b[31;1m%s\x1b[0m\n",err);
}

c**filterOnlyDirs(c**jorked, int*sz) {
	c**res = NULL;
	int count = 0;
	int inpSize = *sz;
	int resSize = 0;
	c full[PATH_MAX] = {0};
	while (count<inpSize) {
		c*unfinished = jorked[count];
		snprintf(full, sizeof(full), "%s/%s", dir, unfinished);
		if ((isDir(full))&&(strcmp(unfinished,".."))) {
			int idx = resSize++;
			res = realloc(res, resSize*sizeof(c*));
			res[idx] = malloc(strlen(unfinished) + 1);
			strcpy(res[idx], unfinished);
		}
		count++;
	}
	*sz = resSize;
	return res;
}

void renderDirPicker(int wd, int hi) {
	size_t count = 0;
	c full[PATH_MAX] = {0};
	c tip[] = "use UP/DOWN 2 navigate, LEFT 2 .., RIGHT to enter, ENTER to submit.";
	bool canTip = wd>=(sizeof(tip)+1);
	int reservedCols = 1+canTip;
	int allowedCols = hi-reservedCols;
	int off = 0;
	if (fileIdx!=0) off=(fileIdx/allowedCols)*allowedCols;
	while (count<hi-reservedCols) {
		if (count>0) puts("\x1b[0m");
		if ((onlyDirsSz==0)&&(count==0)) printf("\x1b[3mmt"); // i print it before any |\x1b[7m|s
		if (count+off==fileIdx) printf("\x1b[7m");
		if (count+off<onlyDirsSz) printf("%s",onlyDirs[count+off]);
		count++;
	}
	puts("\x1b[0m");
	if (canTip) printf("%s\n", tip); // we have enough space for tips!
	c*selected = "";
	if (fileIdx<onlyDirsSz) selected = onlyDirs[fileIdx];
	printf("\x1b[7m%s%s%s\x1b[0m", dir, ((*selected!=0)&&(dir[1]!=0))?"/":"", selected);
}

void setupDirCnsts() {
	renderM = PICK;
	fileIdx = 0;
	dirStuff = jorkdir(dir, &dirStuffSz);
	onlyDirsSz = dirStuffSz;
	onlyDirs = filterOnlyDirs(dirStuff, &onlyDirsSz);
}

int main(int argc, char**argv) {
	dir = getcwd(NULL, 0);
	setupDirCnsts();
	struct termios newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
	switchBufs(2);
	signal(SIGINT, exitHand);
	signal(SIGTERM, exitHand);
	signal(SIGWINCH, winch);
	while (running) {
		if (resized) {
			needsRender = true;
			getTermSize();
		}
		if (needsRender) {
			needsRender = false;
			if ((w<60)||(h<16)) {
				err = "sry, screen too smol :(";
				return 1;
			}
			switch (renderM) {
				case PICK:
				case NONYARGWARN:
					c*pad;
					printf("\x1b[2J\x1b[H");
					c*nls = calloc(h/10+1, sizeof(c)); // +1 for null term
					memset(nls, 10, h/10);
					c*name = "y \x20  a \x20  r \x20  g \x20  i \x20  n \x20  e \x20  !";
					// if (w<44) name = "yargine!";
					if (renderM==NONYARGWARN) name = "warning!!!";
					size_t nameLen = strlen(name);
					size_t padLen = w>nameLen ? (w-strlen(name))/2 : 0;
					pad = calloc(padLen, sizeof(c));
					memset(pad, 32, padLen);
					printf("%s%s\x1b[3%d;1m%s\x1b[0m\n%s", nls, pad, 1+((renderM!=NONYARGWARN)*4), name, nls);
					if (renderM==NONYARGWARN) {
						printf("the folder you selected was detected not a yargine project (or a yargine project for an older version of yargine.)\n");
						printf("please select an action using the highlighted letters, then press enter.\n\n");
						printf("\x1b[7mw\x1b[0mipe contents\n\x1b[7mc\x1b[0mancel");
						break;
					}
					printf("%s  ",nls);
					if (true) printf("\x1b[1myou're not in a yargine project!\x1b[0m ");
					printf("choose project:\n%s", nls);
					renderDirPicker(w,h-((h/10)*4+2));
					free(pad);
					free(nls);
					break;
				case PROJ:
					printf("yargine!");
				default:
					err = "unknown render mode";
					return 1;
			}
			fflush(stdout);
		}
		if (_kbhit()) {
			int initCh = _getch();
			if ((initCh>64)&&(initCh<91)) initCh+=32;
			if (isalpha(initCh)) {
				initCh-=97;
				switch (initCh) {
					case 16:
						running = 0;
						break;
					case 22:
					case 2:
						if (renderM!=NONYARGWARN) break;
						if (initCh==2) {
							dir = getcwd(NULL, 0);
							setupDirCnsts();
							break;
						}
						wipeDir(dir);
						renderM =
						break;
					default: break;
				}
				continue;
			}
			switch (initCh) {
				case 27:
					switch (renderM) {
						case PICK:
							if (_getch()!=91) break;
							int ch = _getch();
							if ((ch<65)||(ch>68)) break;
							needsRender = true;
							if (ch>66) {
								c full[PATH_MAX] = {0};
								c real[PATH_MAX] = {0};
								c*steppingStone;
								c*toSn = "..";
								if (ch==66+1) { // absolutely none of that 69 ripoff in my code
									// this only executes user pressed right btw
									if (onlyDirsSz==0) break; // only run the changing cmd when im not on an empty dir
									toSn = onlyDirs[fileIdx];
								}
								snprintf(full, sizeof(full), "%s/%s", dir, toSn);
								if (realpath(full,real)==NULL) {
									err = "realpath failed";
									return 1;
								}
								steppingStone = calloc(sizeof(c), strlen(real)+1);
								strcpy(steppingStone, real);
								free(dir);
								dir=steppingStone;
								setupDirCnsts();
								break;
							}
							if (onlyDirsSz==0) break;
							ch = 2*ch-131;
							fileIdx += ch;
							while (fileIdx>=onlyDirsSz) fileIdx -= onlyDirsSz;
							while (fileIdx<0) fileIdx += onlyDirsSz;
							break;
						default: break;
					}
					break;
				case 10:
				case 13: // fuckass windows
					if (renderM!=PICK) break; // TODO: swap to switch case like on |case 27:|
					c full[PATH_MAX] = {0};
					snprintf(full, sizeof(full), "%s/%s", dir, onlyDirs[fileIdx]);
					if (realpath(full,real)==NULL) {
						err = "realpath failed";
						return 1;
					}
					steppingStone = calloc(sizeof(c), strlen(real)+1);
					strcpy(steppingStone, real);
					free(dir);
					dir=steppingStone;
					needsRender = true;
					renderM = PROJ;
					break;
				default: break;
			}
		}
	}
	return 0;
}
