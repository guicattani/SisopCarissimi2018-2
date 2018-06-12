#ifndef INODEHANDLER_H
#define INODEHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "t2fs.h"

#define ERROR -1
#define SUCCESS 0

struct t2fs_inode* beingWorkedInode;

bool getInodeToBeingWorkedInode(int inodeIndex);

bool printInodeContentFromBeingWorkedInode();

struct t2fs_record* inodeDataPointerToRecords(DWORD dataPtr);

struct t2fs_record* inodeDataPointerGetFirstRecord(DWORD dataPtr);

bool inodeAppendRecordToBufferBlock(DWORD dataPtr, struct t2fs_record* newRecord, unsigned char bufferBlock[]);

bool prepareNewRecordsBlockBuffer(unsigned char bufferBlock[]);

bool inodeRemoveRecordAndReturnBufferBlock(DWORD dataPtr, char* name, unsigned char bufferBlock[]);

bool inodeFillRecordsToBufferBlock(DWORD dataPtr, unsigned char bufferBlock[]);

#endif
