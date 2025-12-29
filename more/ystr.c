#include "ystr.h"
#include "base.h"
#include "template.h"
yarg thisProj;

bool initYarg(c*name, c*main) {
	memset(full,0,sizeof(full));
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
	memset(real,0,sizeof(real));
	switch (*main) {
		case 47:
			cwk_path_get_relative(dir, main, real, sizeof(real));
			break;
		default:
			strcpy(real, main);
			break;
	}
	// allat to keep the anti else chain goin, worth it?
	len = (uint32_t)strlen(real);
	if (fwrite(&len,sizeof(uint32_t),1,yrFile)!=1) {
		fclose(yrFile);
		return false;
	}
	if (fwrite(real, 1, len, yrFile) != len) {
		fclose(yrFile);
		return false;
	}
	fclose(yrFile);
	memset(full,0,sizeof(full));
	snprintf(full, sizeof(full), "%s/%s", dir, real);
	FILE*fChk = fopen(full, "wb");
	if (fChk) {
		fwrite(template_yrg, 1, template_yrg_len, fChk);
		fclose(fChk);
	}
	return true;
}

// REMEMBER: BEFORE CALLING, ALWAYS SET |projDir|, I BEG
bool readYarg() { // despite it's name, also checks ystr.bin + the main file now!
	memset(full,0,sizeof(full));
	snprintf(full, sizeof(full), "%s/%s", dir, "yarg.bin");
	FILE*ygFile = fopen(full,"rb");
	if (!(ygFile)) return false;
	size_t amnt = fread(&thisProj, sizeof(thisProj), 1, ygFile);
	fclose(ygFile);
	if (amnt!=1) return false;
	if (strncmp(thisProj.hdr,"YARG",4)!=0) return false;
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
	if (strncmp(ystrHdr,"YSTR",4)!=0) return false;
	c*mainFile = readYstr(thisProj.main); // THIS USES |projDir| INSTEAD OF |dir|
	memset(full,0,sizeof(full));
	snprintf(full, sizeof(full), "%s/%s", dir, mainFile);
	free(mainFile);
	FILE*fChk = fopen(full,"r");
	if (!(fChk)) return false;
	fclose(fChk);
	return true;
}

c*readYstr(ystrIdx idx) {
	if (idx==0) return calloc(1,1); // null terminated blank string
	uint32_t strSize;
	ystrIdx zeroBased = idx-1;
	ystrIdx count = 0;
	memset(full,0,sizeof(full));
	snprintf(full, sizeof(full), "%s/%s", projDir, "ystr.bin");
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
	memset(full,0,sizeof(full));
	snprintf(full, sizeof(full), "%s/%s", projDir, "ystr.bin");
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
	memset(full,0,sizeof(full));
	snprintf(full, sizeof(full), "%s/%s", projDir, "ystr.bin");
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