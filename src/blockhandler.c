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

//TODO documentação

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

bool writeBeingWorkedBlockToInodesSector(int blockToBeWrittenIndex) {
    unsigned char buffer[SECTOR_SIZE];
    int index;
    for(index = 0; index < 4; index++) {
        memcpy(&buffer, &beingWorkedBlock[index*SECTOR_SIZE], sizeof(buffer));

        if(write_sector(blockToBeWrittenIndex * controller->boot.blockSize + inodesStartBlock + index, buffer) != 0 )
            return ERROR;
    }

    return SUCCESS;
}

bool writeBeingWorkedBlockToDataSector(int blockToBeWrittenIndex) {
    unsigned char buffer[SECTOR_SIZE];
    int index;
    for(index = 0; index < 4; index++) {
        memcpy(&buffer, &beingWorkedBlock[index*SECTOR_SIZE], sizeof(buffer));

        if(write_sector(blockToBeWrittenIndex * controller->boot.blockSize + blocksStartBlock + index, buffer) != 0 )
            return ERROR;
    }

    return SUCCESS;
}
