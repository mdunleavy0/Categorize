#define _XOPEN_SOURCE 500


#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>


#define CATS_C 9
#define CATS_SZ (CATS_C * sizeof(size_t))


enum CategoryIdx {
	BlockI, CharI, DirI, FileI, LinkI, PipeI, SocketI,
	UnknownI, RestrictI
};


int categorizeDir(char *, size_t *);
int nftwLambda(const char *, const struct stat *, int, struct FTW *);
void printCats(size_t *);
void resetCats(size_t *);


char CatsInitials[] = {'b', 'c', 'd', 'f', 'l', 'p', 's', 'u', '!'};
size_t TmpCats[CATS_C] = {0};


// TODO: parallelise
int main(int argc, char **argv) {
	size_t cats[CATS_C];
	for (int i = 1; i < argc; i++) {
		int err;

		err = categorizeDir(argv[i], cats);

		printf("%s: ", argv[i]);
		if (err)
				printf("Failed");
		else
			printCats(cats);
		printf("\n");
	}

	return 0;
}


int categorizeDir(char *path, size_t *cats) {
	int err;

	resetCats(TmpCats);
	err = nftw(path, nftwLambda, 100, 0);
	if (err)
			return err;

	memcpy(cats, TmpCats, CATS_SZ);
	return 0;
}


// TODO: remove dependance on global TmpCats array
int nftwLambda(const char *path,
	const struct stat *sb,
	int typeFlag,
	struct FTW *ftwBuff) {

	switch (sb->st_mode & S_IFMT) {
		case S_IFBLK:	{ TmpCats[BlockI]++; break; }
		case S_IFCHR:	{ TmpCats[CharI]++; break; }
		case S_IFDIR:	{ TmpCats[DirI]++; break; }
		case S_IFREG:	{ TmpCats[FileI]++; break; }
		case S_IFLNK:	{ TmpCats[LinkI]++; break; }
		case S_IFIFO:	{ TmpCats[PipeI]++; break; }
		case S_IFSOCK:	{ TmpCats[SocketI]++; break; }
		default:		{ TmpCats[UnknownI]++; break; }
	}

	if (typeFlag == FTW_DNR)
			TmpCats[RestrictI]++;
	
	return 0;
}


void printCats(size_t *cats) {
	size_t usedCats[CATS_C];
	char usedInitials[CATS_C];
	int usedC = 0;
	int i;

	for (i = 0; i < CATS_C; i++)
		if (cats[i]) {
			usedCats[usedC] = cats[i];
			usedInitials[usedC] = CatsInitials[i];
			usedC++;
		}

	printf("(");
	for (i = 0; i < usedC - 1; i++)
		printf("%zu%c, ", usedCats[i], usedInitials[i]);
	if (usedC > 0)
		printf("%zu%c", usedCats[i], usedInitials[i]);
	printf(")");
}


void resetCats(size_t *cats) {
	bzero(cats, CATS_SZ);
}
