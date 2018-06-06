#ifndef BLOCKHEADER_H
#define BLOCKHEADER_H

#include <stdbool.h>

#include "t2fs.h"
#include "initializer.h"

#define ERROR -1
#define SUCCESS 0

unsigned char beingWorkedBlock[1024];
unsigned char auxiliaryWorkingBlock[1024];

bool readBlockToBeingWorkedBlock(int blockIndex);
bool readBlockToAuxiliaryWorkingBlock(int blockIndex);
//bool writeBeingWorkedBlockToInodesSection(int blockIndex);
//bool writeBeingWorkedBlockToDataSection(int blockIndex);

bool writeBlockToBlockDataSection(unsigned char* blockToBeWritten, int blockToBeWrittenIndex);
bool writeBlockToInodeDataSection(unsigned char* blockToBeWritten, int blockToBeWrittenIndex);

#endif
