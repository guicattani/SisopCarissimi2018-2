#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <stdbool.h>
#include "t2fs.h"

#define ERROR -1
#define SUCCESS 0

#define N_OPENFILES 20
#define N_OPENDIRECTORIES 50

//typedef struct openFileStruct{
//	int valid;
//	DWORD MFT;
//	DWORD fatherMFT;
//	DWORD currentPointer;
//	DWORD blocksSize;
//	DWORD bytesSize;
//	char name[MAX_FILE_NAME_SIZE];
//
//}t_openfile;

t_control* initializeLibrary();
bool bootFileSystemController();
bool fillBitmaps();
bool initializeDirectories();

#endif
