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
#include "readline/readline.h"
#include "readline/history.h"
#include "cwalk.h" // diabolical

#define _getch getchar
#define YSMAJOR 0 // this should only be changed when the structure's data types are swapped around.

typedef enum {
	PICK,
	NONYARGWARN,
	PROJSETUP,
	PROJ
} RenderMode;

typedef char c;
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
volatile sig_atomic_t running = 1;
volatile sig_atomic_t resized = 1;
volatile sig_atomic_t canceled = 0;
int w;
int h;
int fileIdx;
int dirStuffSz;
bool needsRender;
struct termios oldt;
struct termios newt;
c*err = "";
c*dir;
c*dirB4Enter;
c*rlFill;
c**dirStuff;
c**onlyDirs;
int onlyDirsSz = 0;
RenderMode renderM;
yarg thisProj;

// strings file => "ystr.bin"
// base project => "yarg.bin"

int __readline__startupHook() {
	rl_insert_text(rlFill);
	rl_mark = 0;
	return 0;
}

void afterRl() {
	newt.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	printf("\x1b[?25l");
}

bool isDir(c*p) {
	struct stat st;
	return ((lstat(p, &st) == 0) && (S_ISDIR(st.st_mode)));
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

void setupDirCnsts() {
	renderM = PICK;
	fileIdx = 0;
	dirStuff = jorkdir(dir, &dirStuffSz);
	onlyDirsSz = dirStuffSz;
	onlyDirs = filterOnlyDirs(dirStuff, &onlyDirsSz);
}

void onRlExit() {
	afterRl();
	running = 1;
	free(dir);
	dir = calloc(strlen(dirB4Enter)+1,sizeof(c));
	strcpy(dir,dirB4Enter);
	setupDirCnsts();
	renderM = PICK;
}

bool initYarg(c*name, c*main) {
	c full[PATH_MAX] = {0};
	snprintf(full, sizeof(full), "%s/%s", dir, "yarg.bin");
	FILE*ygFile = fopen(full,"wb");
	if (!(ygFile)) return false;
	memset(&thisProj,0,sizeof(thisProj));
	strncpy(thisProj.hdr, "YARG", 4);
	thisProj.majorVer = YSMAJOR;
	thisProj.projName = 1;
	thisProj.main = 2;
	// TODO: more fields
	if (fwrite(&thisProj,sizeof(thisProj),1,ygFile)!=1) {
		fclose(ygFile);
		return false;
	}
	fclose(ygFile);
	memset(full,0,sizeof(full));
	snprintf(full, sizeof(full), "%s/%s", dir, "ystr.bin");
	/*
		FILE*fChk = fopen(full,"r");
		if (fChk) { // i'm doin ts cuz theres no wb + x
			fclose(fChk);
			return false;
		}
	*/
	// save overhead every day™
	FILE*yrFile = fopen(full,"wb");
	if (!(yrFile)) return false;
	fwrite("YSTR", 1, 4, yrFile);
	uint32_t len = (uint32_t)strlen(name);
	if (fwrite(&len,sizeof(uint32_t),1,yrFile)!=1) {
		fclose(yrFile);
		return false;
	}
	if (fwrite(name, 1, len, yrFile) != len) {
		fclose(yrFile);
		return false;
	}
	memset(full,0,sizeof(full));
	snprintf(full, sizeof(full), "%s/%s", dir, main);
	len = (uint32_t)strlen(full);
	if (fwrite(&len,sizeof(uint32_t),1,yrFile)!=1) {
		fclose(yrFile);
		return false;
	}
	if (fwrite(full, 1, len, yrFile) != len) {
		fclose(yrFile);
		return false;
	}
	fclose(yrFile);
	return true;
}

bool readYarg() { // despite it's name, also checks ystr.bin now!
	c full[PATH_MAX] = {0};
	snprintf(full, sizeof(full), "%s/%s", dir, "yarg.bin");
	FILE*ygFile = fopen(full,"rb");
	if (!(ygFile)) return false;
	size_t amnt = fread(&thisProj, sizeof(thisProj), 1, ygFile);
	fclose(ygFile);
	if (amnt!=1) return false;
	yHdr ystrHdr;
	memset(full,0,sizeof(full));
	snprintf(full, sizeof(full), "%s/%s", dir, "ystr.bin");
	FILE*yrFile = fopen(full,"rb");
	if (!(yrFile)) return false;
	if (fread(ystrHdr, 4, 1, yrFile)!=1) {
		fclose(yrFile);
		return false;
	}
	fclose(yrFile);
	return ((strncmp(thisProj.hdr,"YARG",4)==0)&&(strncmp(ystrHdr,"YSTR",4)==0));
}

c*readYstr(ystrIdx idx) {
	if (idx==0) return calloc(1,1); // null terminated blank string
	uint32_t strSize;
	ystrIdx zeroBased = idx-1;
	ystrIdx count = 0;
	c full[PATH_MAX] = {0};
	snprintf(full, sizeof(full), "%s/%s", dir, "ystr.bin");
	FILE*yrFile = fopen(full,"rb");
	if (!(yrFile)) return NULL;
	yHdr ystrHdr;
	if (fread(ystrHdr, sizeof(yHdr), 1, yrFile)!=1) {
		fclose(yrFile);
		return NULL; // tehe segfault~ >:3
	}
	if (strncmp(ystrHdr,"YSTR",4)!=0) {
		fclose(yrFile);
		return NULL;
	}
	while (count<zeroBased) {
		if (fread(&strSize, sizeof(uint32_t), 1, yrFile)!=1) {
			fclose(yrFile);
			return NULL;
		}
		if (fseek(yrFile, strSize, SEEK_CUR)!=0) {
			fclose(yrFile);
			return NULL;
		}
		count++;
	}
	if (fread(&strSize, sizeof(uint32_t), 1, yrFile)!=1) {
		fclose(yrFile);
		return NULL;
	}
	c*res = calloc(strSize+1,sizeof(c)); // strSize excludes null term, string itself 2!
	if (!(res)) {
		fclose(yrFile);
		return NULL;
	}
	if (fread(res, strSize, 1, yrFile)!=1) {
		fclose(yrFile);
		return NULL;
	}
	fclose(yrFile);
	return res;
}

ystrIdx genYstr(c*new) {
	if (!(new)) return 0;
	if (*new==0) return 0;
	uint32_t strSize;
	// ystr indices always start from 1
	ystrIdx count = 0;
	c full[PATH_MAX] = {0};
	snprintf(full, sizeof(full), "%s/%s", dir, "ystr.bin");
	FILE*yrFile = fopen(full,"rb");
	if (!(yrFile)) return 0;
	yHdr ystrHdr;
	if (fread(ystrHdr, sizeof(yHdr), 1, yrFile)!=1) {
		fclose(yrFile);
		return 0;
	}
	if (strncmp(ystrHdr,"YSTR",4)!=0) {
		fclose(yrFile);
		return 0;
	}
	while (!(feof(yrFile))) {
		if (fread(&strSize, sizeof(uint32_t), 1, yrFile)!=1) {
			fclose(yrFile);
			return 0;
		}
		if (fseek(yrFile, strSize, SEEK_CUR)!=0) { // 0 reads here...
			fclose(yrFile);
			return 0;
		}
		count++; // :p
	}
	fclose(yrFile);
	yrFile = fopen(full,"ab");
	uint32_t lengthOfTS = (uint32_t)strlen(new); // snatched from modifyYstr lmao
	fwrite(&lengthOfTS, sizeof(uint32_t), 1, yrFile);
	fwrite(new, lengthOfTS, 1, yrFile);
	fclose(yrFile);
	return count+1;
}

bool modifyYstr(ystrIdx idx, c*new) {
	if (idx==0) return false;
	uint32_t strSize;
	ystrIdx zeroBased = idx-1;
	c*beforeIdx=calloc(sizeof(yHdr)+1,1); // hehy null term
	if (!(beforeIdx)) return false;
	ystrIdx count = 0;
	c full[PATH_MAX] = {0};
	snprintf(full, sizeof(full), "%s/%s", dir, "ystr.bin");
	FILE*yrFile = fopen(full,"rb");
	if (!(yrFile)) return false;
	if (fread(beforeIdx, sizeof(yHdr), 1, yrFile)!=1) {
		fclose(yrFile);
		return false;
	}
	if (strncmp(beforeIdx,"YSTR",4)!=0) {
		fclose(yrFile);
		return false;
	}
	while (count<zeroBased) {
		if (fread(&strSize, sizeof(uint32_t), 1, yrFile)!=1) {
			fclose(yrFile);
			free(beforeIdx);
			return false;
		}
		c*add = calloc(sizeof(c),strSize+1);
		if (!(add)) {
			fclose(yrFile);
			free(beforeIdx);
		}
		if (fread(add, strSize, 1, yrFile)!=1) {
			fclose(yrFile);
			free(beforeIdx);
			free(add);
			return false;
		}
		size_t prevSize = strlen(beforeIdx);
		beforeIdx = realloc(beforeIdx,prevSize+sizeof(uint32_t)+strSize+1); // null term
		if (!(beforeIdx)) { // YLTPYL PART 1 (TRY NOT TO LEAK MEMORY!!!)
			fclose(yrFile);
			free(add);
			return false;
		}
		memcpy(beforeIdx+prevSize, &strSize, sizeof(uint32_t));
		memcpy(beforeIdx+prevSize+sizeof(uint32_t), add, strSize);
		beforeIdx[prevSize+sizeof(uint32_t)+strSize]=0;
		free(add);
		count++;
	}
	if (fread(&strSize, sizeof(uint32_t), 1, yrFile)!=1) {
		fclose(yrFile);
		free(beforeIdx);
		return false;
	}
	if (fseek(yrFile, strSize, SEEK_CUR)!=0) {
		fclose(yrFile);
		free(beforeIdx);
		return false;
	}
	long int curr = ftell(yrFile);
	fseek(yrFile, 0, SEEK_END);
	long int end = ftell(yrFile);
	fseek(yrFile, curr, SEEK_SET); // get yo goofy ahh back here
	size_t remaining = end - curr;
	c*afterIdx = calloc(sizeof(c),remaining+1);
	if (!(afterIdx)) {
		fclose(yrFile);
		free(beforeIdx);
		return false;
	}
	if (fread(afterIdx, remaining, 1, yrFile)!=1) {
		fclose(yrFile);
		free(beforeIdx);
		free(afterIdx);
		return false;
	}
	fclose(yrFile);
	yrFile = fopen(full,"wb");
	// get ready for satisfying freeing...
	fwrite(beforeIdx, strlen(beforeIdx), 1, yrFile);
	free(beforeIdx); // mmhh
	// WE INTERRUPT THIS PROGRAM WITH AN INBETWEEN WRITING WITH NO MALLOCS OR FREEING!!!
	uint32_t lengthOfTS = (uint32_t)strlen(new);
	fwrite(&lengthOfTS, sizeof(uint32_t), 1, yrFile);
	fwrite(new, lengthOfTS, 1, yrFile);
	// we will now go back to our regular programming...
	fwrite(afterIdx, remaining, 1, yrFile);
	free(afterIdx); // "so gooood..." - the literal system
	fclose(yrFile); // "do i even deserve this..." - system
	return true; // thank for watching like and subcrib
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
	canceled = 1;
	rl_replace_line("", 0);
	rl_point = 0;
	rl_done = 1;
	rl_pending_input = 10;
	ssize_t satisfaction = write(1, "\n", 1); // shut up c
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
	fflush(stdout);
	switchBufs(1);
	oldt.c_lflag |= ICANON | ECHO; // in case i fucked up someone's terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) & ~O_NONBLOCK);
	free(dir);
	free(dirB4Enter);
	freeJorked(dirStuff, dirStuffSz);
	freeJorked(onlyDirs, onlyDirsSz);
	if (*err==0) return;
	printf("\x1b[31;1m%s\x1b[0m\n",err);
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
		if (count>0) printf("\n\x1b[0m");
		if ((onlyDirsSz==0)&&(count==0)) printf("\x1b[3mmt"); // i print it before any |\x1b[7m|s
		if (count+off==fileIdx) printf("\x1b[7m");
		if (count+off<onlyDirsSz) printf("%-*s",wd,onlyDirs[count+off]);
		count++;
	}
	puts("\x1b[0m");
	if (canTip) printf("%s\n", tip); // we have enough space for tips!
	c*selected = "";
	if (onlyDirsSz>0) selected = onlyDirs[fileIdx];
	printf("\x1b[7m%s%s%s\x1b[0m", dir, ((*selected!=0)&&(dir[1]!=0))?"/":"", selected);
}

int main(int argc, char**argv) {
	bool cwdValid = readYarg();
	dirB4Enter = malloc(1); // 100% freeable
	dir = getcwd(NULL, 0);
	setupDirCnsts();
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
	switchBufs(2);
	signal(SIGINT, exitHand);
	signal(SIGTERM, exitHand);
	signal(SIGWINCH, winch);
	rl_startup_hook = __readline__startupHook;
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
			printf("\x1b[2J\x1b[H");
			switch (renderM) {
				case PICK:
				case NONYARGWARN:
					printf("\x1b]0;%s!\x1b\\", (renderM==NONYARGWARN)?"warning!!":"yargine");
					c*pad;
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
						printf("the folder you selected was detected not a yargine project (or a yargine project for an older/newer version of yargine.)\n");
						printf("please select an action using the highlighted letters, then press enter.\n\n");
						printf("\x1b[7mw\x1b[0mipe contents\n\x1b[7mc\x1b[0mancel");
						break;
					}
					printf("%s  ",nls);
					if (!(cwdValid)) printf("\x1b[1myour cwd isn't a valid yargine project!\x1b[0m ");
					printf("choose project:\n%s", nls);
					renderDirPicker(w,h-((h/10)*4+2));
					free(pad);
					free(nls);
					break;
				case PROJSETUP:
					printf("\x1b]0;yargine!\x1b\\");
					printf("the folder you selected was eligible for a new yargine project or got fully wiped.\nplease take your time to set up your new project.\nnote: do press enter after pressing ^c. idk why. just do.\n\n\x1b[?25h");
					needsRender = true;
					newt.c_lflag |= ECHO;
					tcsetattr(STDIN_FILENO, TCSANOW, &newt);
					canceled = 0;
					rlFill = strrchr(dir,47)+1;
					c*pName = readline("project name: ");
					if ((canceled)||((!(pName))||(*pName==0))) {
						onRlExit();
						break;
					}
					rlFill = "";
					c*main = readline("main file: ");
					if ((canceled)||((!(main))||(*main==0))) {
						onRlExit();
						break;
					}
					initYarg(pName, main);
					renderM = PROJ;
					break;
				case PROJ:
					pName = readYstr(thisProj.projName);
					printf("\x1b]0;%s - yargine!\x1b\\\x1b[3m%s\x1b[0m - yargine!", pName, pName);
					free(pName);
					break;
				default:
					err = "unknown render mode";
					return 1;
			}
			fflush(stdout);
		}
		if ((_kbhit())&&(running)) {
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
						needsRender = true;
						if (initCh==2) {
							free(dir);
							dir = calloc(strlen(dirB4Enter)+1,sizeof(c));
							strcpy(dir,dirB4Enter);
							setupDirCnsts();
							break;
						}
						wipeDir(dir);
						renderM = PROJSETUP;
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
								c*steppingStone = calloc(sizeof(c), strlen(real)+1);
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
					dirB4Enter = calloc(strlen(dir)+1,sizeof(c));
					strcpy(dirB4Enter,dir);
					c full[PATH_MAX] = {0};
					c real[PATH_MAX] = {0};
					c*chosen = ".";
					if (onlyDirsSz>0) chosen=onlyDirs[fileIdx];
					snprintf(full, sizeof(full), "%s/%s", dir, chosen);
					if (realpath(full,real)==NULL) {
						err = "realpath failed";
						return 1;
					}
					c*steppingStone = calloc(sizeof(c), strlen(real)+1);
					strcpy(steppingStone, real);
					free(dir);
					dir=steppingStone;
					needsRender = true;
					bool yargValid = readYarg();
					/*
						snprintf(full, sizeof(full), "%s/%s", dir, "yarg.bin");
						FILE*fChk1 = fopen(full,"r");
						snprintf(full, sizeof(full), "%s/%s", dir, "ystr.bin");
						FILE*fChk2 = fopen(full,"r");
						bool filesExist = (fChk1)&&(fChk2);
						if (fChk1) fclose(fChk1);
						if (fChk2) fclose(fChk2);
					*/
					// day 2 of saving overhead every day™ (trust)
					int isNonEmpty = 0;
					c**jorked = jorkdir(dir, &isNonEmpty);
					freeJorked(jorked, isNonEmpty); // instantaneously free the jorked
					if (isNonEmpty>=1) isNonEmpty--; // exclude the |..| (there is no |.| in |jorkdir|)
					if ((!(yargValid))&&(isNonEmpty)) {
						renderM = NONYARGWARN;
						break;
					}
					if ((!(isNonEmpty))&&(!(yargValid))) { // totally blank
						renderM = PROJSETUP;
						break;
					}
					if ((yargValid)&&(isNonEmpty)) { // explicitly check if non empty...
						renderM = PROJ;
						break;
					}
					err = "yargine files valid but basically nonexistent?? howz???"; // ...to let this happen someday
					return 1;
				default: break;
			}
		}
	}
	return 0;
}
