#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

#include "../include/initializer.h"
#include "../include/blockhandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int blockSize;

int freeBlocksBitmapSizeInSectors;
int freeInodesBitmapSizeInSectors;
int inodesAreaSizeInSectors;
int diskSizeInSectors;
int blocksAreaSizeInSectors;

int inodesStartSector;
int inodesStartBlock;
int blocksStartSector;
int blocksStartBlock;

bool initializeBeingWorkedBlock(t_control* controller){
    blockSize                     = controller->boot.blockSize;

    freeBlocksBitmapSizeInSectors = controller->boot.freeBlocksBitmapSize * blockSize;
    freeInodesBitmapSizeInSectors = controller->boot.freeInodeBitmapSize * blockSize;
    inodesAreaSizeInSectors       = controller->boot.inodeAreaSize * blockSize;
    diskSizeInSectors             = controller->boot.diskSize * blockSize;
    blocksAreaSizeInSectors       = diskSizeInSectors
                                    - controller->boot.superblockSize* blockSize
                                    - freeBlocksBitmapSizeInSectors
                                    - freeInodesBitmapSizeInSectors
                                    - inodesAreaSizeInSectors;

    inodesStartSector = blockSize + freeBlocksBitmapSizeInSectors + freeInodesBitmapSizeInSectors;
    inodesStartBlock  = inodesStartSector / blockSize;
    blocksStartSector = inodesStartSector + inodesAreaSizeInSectors;
    blocksStartBlock  = blocksStartSector / blockSize;

    return SUCCESS;
}

bool readBlockToBeingWorkedBlock(int blockIndex) {
    unsigned char buffer[SECTOR_SIZE];
    memset(buffer, 0, sizeof(buffer));

    int index;
    for(index = 0; index < 4; index++) {
       if(read_sector(blockIndex*blockSize + index, buffer) != 0 )
            return ERROR;
        memcpy(&beingWorkedBlock[index*SECTOR_SIZE], &buffer, sizeof(buffer));
    }
    return SUCCESS;
}

bool readBlockToAuxiliaryWorkingBlock(int blockIndex) {
    unsigned char buffer[SECTOR_SIZE];
    int index;
    for(index = 0; index < 4; index++) {
        if(read_sector(blockIndex*blockSize + index, buffer) != 0 )
            return ERROR;
        memcpy(&auxiliaryWorkingBlock[index*SECTOR_SIZE], &buffer, sizeof(buffer));
    }

    return SUCCESS;
}

bool writeBeingWorkedBlock(int blockIndex) {
    unsigned char buffer[SECTOR_SIZE];
    int index;
    for(index = 0; index < 4; index++) {
        memcpy(&buffer, &beingWorkedBlock[index*SECTOR_SIZE], sizeof(buffer));

        if(write_sector(blockIndex*blockSize + index, buffer) != 0 )
            return ERROR;
    }

    return SUCCESS;
}
