#ifndef BLOCKHEADER_H
#define BLOCKHEADER_H

#include <stdbool.h>

#include "t2fs.h"
#include "initializer.h"

#define ERROR -1
#define SUCCESS 0

unsigned char beingWorkedBlock[1024];
unsigned char auxiliaryWorkingBlock[1024];

bool readBlockToBuffer(int blockIndex, char* buffer);

bool readBlockToBeingWorkedBlock(int blockIndex);

bool readBlockToAuxiliaryWorkingBlock(int blockIndex);

bool writeInodeToInodeDataSection(struct t2fs_inode* inode, int inodeIndex);

bool writeBlockToBlockDataSection(unsigned char* blockToBeWritten, int blockToBeWrittenIndex);

bool readAdressesToDataPointerCollection (DWORD* collection, DWORD adress, int amountOfDesiredAdresses);

#endif
