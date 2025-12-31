#include "base.h"
#include "ystr.h"
#include "picker.h"

#define _getch getchar // i should prob go shit myself

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
c*dir;
c*dirB4Enter;
c*rlFill;
c**dirStuff;
RenderMode renderM;
bool cwdValid;
c*projDir;
PViewMode pvMode;
c**vp2Str = (c*[]){
	[INTRO] = "welkommen",
	[SETT] = "settinms",
	[EXPT] = "export",
	[YRGS] = "yargine settinms"
};
c**vp2Ch = (c*[]){
	[SETT] = "s",
	[EXPT] = "e",
	[YRGS] = "y"
};

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

c**filterJorked(c**jorked, int*sz, bool filesToo, bool dotdot) { // only point is: yank away |..|
	c**res = NULL;
	int count = 0;
	int inpSize = *sz;
	int resSize = 0;
	memset(full,0,sizeof(full));
	while (count<inpSize) {
		c*unfinished = jorked[count];
		snprintf(full, sizeof(full), "%s/%s", dir, unfinished);
		bool can = filesToo?true:isDir(full);
		if ((can)&&((strcmp(unfinished,"..")==0)==dotdot)) { // xnor
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
	int unfitSz;
	c**unfit = jorkdir(dir, &unfitSz);
	dirStuffSz = unfitSz;
	dirStuff = filterJorked(unfit, &dirStuffSz, false, false);
	freeJorked(unfit, unfitSz);
}

void resetDirForPROJ() {
	freeJorked(dirStuff, dirStuffSz);
	int unfitSz;
	c**unfit = jorkdir(dir, &unfitSz);
	dirStuffSz = unfitSz;
	dirStuff = filterJorked(unfit, &dirStuffSz, true, strcmp(projDir,dir)!=0);
	freeJorked(unfit, unfitSz);
}

void onRlExit() {
	afterRl();
	running = 1;
	free(dir);
	dir = calloc(strlen(dirB4Enter)+1,sizeof(c));
	strcpy(dir,dirB4Enter);
	freeJorked(dirStuff, dirStuffSz);
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
	if (*err==0) return;
	printf("\x1b[31;1m%s\x1b[0m\n",err);
}

int main(int argc, c**argv) {
	cwk_path_set_style(CWK_STYLE_UNIX); // i dont think im swallowing the windows pill anytime soon
	dirB4Enter = malloc(1); // 100% freeable
	dir = getcwd(NULL, 0);
	projDir = dir;
	cwdValid = readYarg(); // check cwd when |dir| & |projDir| are there
	if (cwdValid) {
		resetDirForPROJ();
		initPvMode();
		renderM = PROJ;
	}
	if (renderM != PROJ) {
		projDir = malloc(1);
		setupDirCnsts();
	}
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
					renderRoot(renderM==PICK);
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
					printf("\x1b]0;%s - yargine!\x1b\\\x1b[3m%s\x1b[0m - yargine!\n", pName, pName);
					free(pName);
					// w/2 <= w-(w/2)
					int editorW = w/2;
					renderPicker(editorW,h-1);
					renderPROJ_View(editorW,1,w-editorW,h-1);
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
				if (renderM==PROJ) {
					size_t cnt1 = 0;
					bool didFind = false;
					while (cnt1<PVIEWS) {
						if ((vp2Ch[cnt1]!=NULL)&&(*vp2Ch[cnt1]!=0)) {
							c idek = *vp2Ch[cnt1];
							if ((idek>64)&&(idek<91)) idek+=32; // match initCh
							if (*vp2Ch[cnt1]==initCh) {
								pvMode = cnt1;
								didFind = true;
								needsRender = true;
								break;
							}
						}
						cnt1++;
					}
					if (didFind) continue; // whatever dude
				}
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
							freeJorked(dirStuff, dirStuffSz);
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
						case PROJ:
						case PICK:
							if (_getch()!=91) break;
							int ch = _getch();
							if ((ch<65)||(ch>68)) break;
							needsRender = true;
							if (ch>66) {
								switch (renderM) {
									case PICK:
										memset(full,0,sizeof(full));
										memset(real,0,sizeof(real));
										c*toSn = "..";
										if (ch==66+1) { // absolutely none of that 69 ripoff in my code
											// this only executes user pressed right btw
											if (dirStuffSz==0) break; // only run the changing cmd when im not on an empty dir
											toSn = dirStuff[fileIdx];
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
										freeJorked(dirStuff, dirStuffSz);
										setupDirCnsts();
										break;
									case PROJ:
										int incr = 1;
										if (ch==68) incr = -1;
										pvMode = (pvMode+incr+PVIEWS)%(PVIEWS);
										break;
									default:
										__builtin_unreachable(); // neato
										break;
								}
								break;
							}
							if (dirStuffSz==0) break;
							ch = 2*ch-131;
							fileIdx += ch;
							while (fileIdx>=dirStuffSz) fileIdx -= dirStuffSz;
							while (fileIdx<0) fileIdx += dirStuffSz;
							break;
						default: break;
					}
					break;
				case 10:
					if ((renderM!=PICK)&&(renderM!=PROJ)) break;
					if (renderM==PICK) {
						dirB4Enter = calloc(strlen(dir)+1,sizeof(c));
						strcpy(dirB4Enter,dir);
					}
					memset(full,0,sizeof(full));
					memset(real,0,sizeof(real));
					c*chosen = ".";
					if (dirStuffSz>0) chosen=dirStuff[fileIdx];
					snprintf(full, sizeof(full), "%s/%s", dir, chosen);
					if (realpath(full,real)==NULL) {
						err = "realpath failed";
						return 1;
					}
					c*steppingStone = calloc(sizeof(c), strlen(real)+1);
					strcpy(steppingStone, real);
					needsRender = true; // yes, even if |renderM==PROJ| and |steppingStone| isn't a dir.
					if (renderM==PROJ) {
						if (isDir(steppingStone)) {
							fileIdx = 0;
							free(dir);
							dir=steppingStone;
							resetDirForPROJ();
							break;
						}
						openFileWithGUI(steppingStone);
						free(steppingStone);
						break;
					}
					free(dir);
					dir=steppingStone;
					projDir = calloc(strlen(dir)+1,sizeof(c));
					strcpy(projDir,dir);
					bool yargValid = readYarg();
					// there was a massive section of code here
					// day 2 of saving overhead every day™ (trust)
					int isNonEmpty = 0;
					c**jorked = jorkdir(dir, &isNonEmpty);
					freeJorked(jorked, isNonEmpty); // instantaneously free the jorked
					if (isNonEmpty>=1) isNonEmpty--; // exclude the |..| (there is no |.| in |jorkdir|)
					fileIdx = 0;
					if ((!(yargValid))&&(isNonEmpty)) {
						renderM = NONYARGWARN;
						break;
					}
					if ((!(isNonEmpty))&&(!(yargValid))) { // totally blank
						renderM = PROJSETUP;
						break;
					}
					if ((yargValid)&&(isNonEmpty)) { // explicitly check if non empty...
						resetDirForPROJ();
						initPvMode();
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