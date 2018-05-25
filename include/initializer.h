/*
 * initializer.h: Responsible for initializing the library
 */

#include <stdbool.h>
#include "t2fs.h"

#define ERROR -1;
#define SUCCESS 0;

#define N_OPENFILES 20
#define N_OPENDIRECTORIES 50

typedef struct openFile{
	int valid;
	DWORD MFT;
	DWORD fatherMFT;
	DWORD currentPointer;
	DWORD blocksSize;
	DWORD bytesSize;
	char name[MAX_FILE_NAME_SIZE];

}OPENFILE;

typedef struct openDirectory{
	int valid;
	DWORD MFT;
	DWORD currentEntry;
	DWORD blocksSize;
	DWORD bytesSize;
	char name[MAX_FILE_NAME_SIZE];

}OPENDIRECTORY;

typedef struct s_Control {

	struct t2fs_superbloco boot;
	OPENFILE openFilesArray[N_OPENFILES];
	OPENDIRECTORY openDirectoriesArray[N_OPENDIRECTORIES];
	int N_OpenFiles;
	int N_OpenDirectories;

} t_control;


t_control* initializeLibrary();
bool bootFileSystem();
