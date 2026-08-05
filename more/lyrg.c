#include "base.h"
#include "lyrg.h"

static c*__inp;
static c lyrg__err[128];
static size_t lyrg__line = 1;
static struct LyrgStat {
	bool ok;
	void*res;
} lyrg__stat;
static c*ops = "+-*/";

static LyrgStat lyrg_kms(c*, LyrgStat);
static bool lyrg__isNL(c);
static bool lyrg__isWS(c);
static void lyrg__slurpWS();
static void lyrg__slurpComment();
static void lyrg__parseExpression();

static void lyrg__kms(c*m) {
	size_t col = 1;
	c*tmp = __inp;
	while (!(lyrg__isNL(*tmp))) {
		tmp--;
		col++;
	}
	snprintf(lyrg__err, sizeof(lyrg__err), "%s@%zu:%zu: %s", "<input>", lyrg__line, col, m);
	lyrg__stat.ok = false;
	lyrg__stat.res = lyrg__err;
	return lyrg__stat;
}

static bool lyrg__isNL(c ch) {
	return (strchr("\r\n",ch)!=NULL);
}

static bool lyrg__isWS(c ch) { 
	return ((strchr("\x20\t_",ch)!=NULL) || (lyrg__isNL(ch)));
}

static void lyrg__slurpWS() {
	while (lyrg__isWS(*__inp)) {
		if (lyrg__isNL(*__inp)) {
			if (strncmp(__inp,"\r\n",2)==0) __inp++;
			lyrg__line++;
		}
		__inp++;
	}
}

static void lyrg__parseExpression() {
	if (strchr(ops,ch)!=NULL) {
		lyrg__stat.ok = true;
		lyrg__stat.res = calloc(1,sizeof(LyrgCmd));
		((LyrgCmd)(lyrg__stat.res)).type = __inp[0];
		if not lyrg__isWS(__inp[1]) lyrg__kms("expected whitespace")
		inp += 2; // day 4 of saving overhead every day™ (trust)
		lyrg__slurpWS();
		lyrg__parseExpression();
		if (!(lyrg__stat.ok)) return;
		lyrg__parseExpression();
		if (!(lyrg__stat.ok)) return;
	}
}

static void lyrg__slurpComment() {
	c*firstNl = strchr(__inp,10);
	c*firstCr = strchr(__inp,13);
	if ((firstNl==NULL) && (firstCr==NULL)) {
		__inp=strchr(__inp,0); // idek what am i doing i just hope it passes -Werror lmfao
		return;
	}
	c*won = NULL;
	if (firstNl == NULL) won = firstCr;
	if (firstCr == NULL) won = firstNl;
	if (won==NULL) won = (firstNl<firstCr) ? firstNl : firstCr;
	if (firstCr+1 == firstNl) won = firstNl; // handle \r\n EVEN BETTAR!!
	__inp = won+1;
	lyrg__line++;
	lyrg__slurpWS();
}

LyrgRes*parseLyrg(c*inp) {
	LyrgRes*o = calloc(1,sizeof(LyrgRes));
	o->err = "testing"; // if NULL, caller should check chain. if chain is also NULL, treat as empty file.
	o->chain = NULL;
	__inp=inp;
	if (strncmp(__inp,"#!",2)==0) lyrg__slurpComment();
	return o;
}