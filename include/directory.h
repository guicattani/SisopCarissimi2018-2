#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdbool.h>
#include "t2fs.h"

#define ERROR -1
#define SUCCESS 0

void printDirectoryTree();

void printerOfTree(int level, int inodeNumber) ;

void printSubDirectories(int level, DWORD dataPointer);

bool relativePathExists(char* path, struct t2fs_record* directory);

bool absolutePathExists();

//upperDirectoryInode should be first used with -1
void getPathToDirectory(struct t2fs_record* directory, char* completePath, int upperDirectoryInode);

#endif
