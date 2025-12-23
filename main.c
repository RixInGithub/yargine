#include "base.h"
#include "ystr.h"
#include "picker.h"

#include "jorkdir.h"
#include "binaryen-c.h"
#include "readline/readline.h"
#include "readline/history.h"
#include "cwalk.h" // diabolical

#define _getch getchar

typedef enum {
	PICK,
	NONYARGWARN,
	PROJSETUP,
	PROJ
} RenderMode;

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
c*dirB4Enter;
c*rlFill;
c**dirStuff;
c**onlyDirs;
int onlyDirsSz = 0;
RenderMode renderM;
bool cwdValid;

// strings file => "ystr.bin"
// base project => "yarg.bin"

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

c**filterOnlyDirs(c**jorked, int*sz) {
	c**res = NULL;
	int count = 0;
	int inpSize = *sz;
	int resSize = 0;
	memset(full,0,sizeof(full));
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

void getTermSize() {
	if (!(resized)) return;
	resized = 0;
	struct winsize ws;
	if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws)==-1){w=0;h=0;return;}
	w=ws.ws_col;
	h=ws.ws_row;
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

int main(int argc, char**argv) {
	cwdValid = readYarg();
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
					renderPicker(renderM==PICK);
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
								memset(full,0,sizeof(full));
								memset(real,0,sizeof(real));
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
					memset(full,0,sizeof(full));
					memset(real,0,sizeof(real));
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
					// there was a massive section of code to check if yarg.bin + ystr.bin for a single bool on here, but i decided to snip it off for the lines.
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