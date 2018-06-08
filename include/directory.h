#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdbool.h>
#include "t2fs.h"

#define ERROR -1
#define SUCCESS 0

#define MAX_PATH_LENGTH 59

void printDirectoryTree();

void printerOfTree(int level, int inodeNumber) ;

void printSubDirectories(int level, DWORD dataPointer);

struct t2fs_record* relativePathExists(char* path, struct t2fs_record* directory, int typeValOfTarget);

struct t2fs_record* absolutePathExists(char* path, struct t2fs_record* directory, int typeValOfTarget);

struct t2fs_record* findRecordOfPath(char* path);

struct t2fs_record* findRecordOfFile(char* path);

//upperDirectoryInode should be first used with -1
void getPathToDirectory(struct t2fs_record* directory, char* completePath, int upperDirectoryInode);

struct t2fs_record* returnRecordOfParentDirectory(char* path);

bool getNameOfDirectoryAtEndOfPath(char* path, char* name);

#endif
