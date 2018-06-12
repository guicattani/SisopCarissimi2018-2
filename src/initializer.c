#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

#include "../include/initializer.h"
#include "../include/directory.h"
#include "../include/inodehandler.h"
#include "../include/blockhandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//TODO documentação

t_control* initializeLibrary() {
    controller = malloc(sizeof(t_control));
    rootDirectory =  malloc(sizeof(struct t2fs_record));
    currentDirectory = malloc(sizeof(struct t2fs_record));

    beingWorkedRecord = malloc(sizeof(struct t2fs_record));
    beingWorkedInode = malloc(sizeof(struct t2fs_inode));

    struct t2fs_record* record;

    bool errorCode;
    errorCode = bootFileSystemController();
    if(errorCode) {
        fprintf(stderr, "!FATAL! // initializeLibrary // BOOT FAILED\n");
        return NULL;
    }

    errorCode = initializeOpenedDirectories();
    if(errorCode) {
        fprintf(stderr, "!FATAL! // initializeLibrary // BOOT FAILED\n");
        return NULL;
    }

    errorCode = initializeOpenedFiles();
    if(errorCode) {
        fprintf(stderr, "!FATAL! // initializeLibrary // BOOT FAILED\n");
        return NULL;
    }

    errorCode = initializeGlobalVariables();
    if(errorCode) {
        fprintf(stderr, "!FATAL! // initializeLibrary // BOOT FAILED\n");
        return NULL;
    }

    getInodeToBeingWorkedInode(0);
    record = inodeDataPointerGetFirstRecord(beingWorkedInode->dataPtr[0]);

    *rootDirectory = *record;
    *currentDirectory = *record;

    return controller;
}

bool bootFileSystemController()
{
    unsigned char buffer[256];
    int index = 0;

    if(read_sector(0, buffer) != 0) {
        fprintf(stderr, "!ERROR! // bootFileSystemController // failed reading sector 0\n");
        return ERROR;
    }

    memcpy(controller->boot.id,buffer,4*sizeof(char));
    index += 4*sizeof(char);
    memcpy(&controller->boot.version,buffer+index,sizeof(WORD));
    index += sizeof(WORD);
    memcpy(&controller->boot.superblockSize,buffer+index,sizeof(WORD));
    index += sizeof(WORD);
    memcpy(&controller->boot.freeBlocksBitmapSize,buffer+index,sizeof(WORD));
    index += sizeof(WORD);
    memcpy(&controller->boot.freeInodeBitmapSize,buffer+index,sizeof(WORD));
    index += sizeof(WORD);
    memcpy(&controller->boot.inodeAreaSize,buffer+index,sizeof(WORD));
    index += sizeof(WORD);
    memcpy(&controller->boot.blockSize,buffer+index,sizeof(WORD));
    index += sizeof(WORD);
    memcpy(&controller->boot.diskSize,buffer+index,sizeof(DWORD));

//    printf("Boot Data:\n");
//    printf("\tid:%c%c%c%c\n\tversion:%u\n\tsuperblockSize1:%u\n\tfreeBlocksBitmapSize:%u\n\tfreeInodeBitmapSize:%u\n\tinodeAreaSize:%u\n\tblockSize:%u\n\tdiskSize:%u\n",
//           controller->boot.id[0],
//           controller->boot.id[1],
//           controller->boot.id[2],
//           controller->boot.id[3],
//           controller->boot.version,
//           controller->boot.superblockSize,
//           controller->boot.freeBlocksBitmapSize,
//           controller->boot.freeInodeBitmapSize,
//           controller->boot.inodeAreaSize,
//           controller->boot.blockSize,
//           controller->boot.diskSize);

    if(controller->boot.version != 32289) {
        fprintf(stderr, "!ERROR! // bootFileSystemController // file system is outdated\n");
        return ERROR;
    }

    if(controller->boot.id[0] != 'T' || controller->boot.id[1] != '2' || controller->boot.id[2] != 'F' || controller->boot.id[3] != 'S') {
        fprintf(stderr, "!ERROR! // bootFileSystemController // file system id is invalid\n");
        return ERROR;
    }

    return SUCCESS;
}

bool fillBitmaps() {

    if(controller == NULL)
    {
        fprintf(stderr, "!ERROR! // fillBitmaps // controller NULL\n");
        return ERROR;
    }

    unsigned char buffer[SECTOR_SIZE];

    if(read_sector(1, buffer) != 0) {
        fprintf(stderr, "!ERROR! // fillBitmaps // error reading sector 1\n");
        return ERROR;
    }

    controller->freeBlockBitmap = (int) buffer;

    if(read_sector(1, buffer) != 0) {
        fprintf(stderr, "!ERROR! // fillBitmaps // error reading sector 1\n");
        return ERROR;
    }

    controller->freeInodeBitmap = (int) buffer;

    return SUCCESS;
}

bool initializeOpenedDirectories() {
    int index;
    for(index = 0; index < MAX_OPEN_DIRECTORIES; index++) {
        openedDirectories[index].valid = false;
    }
    return SUCCESS;
}

bool initializeOpenedFiles() {
    int index;
    for(index = 0; index < MAX_OPEN_FILES; index++) {
        openedFiles[index].valid = false;
    }
    return SUCCESS;
}


bool initializeGlobalVariables(){
    int blockSize                         = controller->boot.blockSize;

    int freeBlocksBitmapSizeInSectors = controller->boot.freeBlocksBitmapSize * blockSize;
    int freeInodesBitmapSizeInSectors = controller->boot.freeInodeBitmapSize * blockSize;
    int inodesAreaSizeInSectors       = controller->boot.inodeAreaSize * blockSize;

    int inodesStartSector = blockSize + freeBlocksBitmapSizeInSectors + freeInodesBitmapSizeInSectors;
    inodesStartBlock  = inodesStartSector / blockSize;
    int blocksStartSector = inodesStartSector + inodesAreaSizeInSectors;
    blocksStartBlock  = blocksStartSector / blockSize;

    inodesSectionInBlocks  = blocksStartBlock - inodesStartBlock;
    dataSectionInBlocks  = controller->boot.diskSize -blocksStartBlock;

    return SUCCESS;
}

