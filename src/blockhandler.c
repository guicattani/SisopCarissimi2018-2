#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

#include "../include/initializer.h"
#include "../include/blockhandler.h"
#include "../include/inodehandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
        if(read_sector(blockIndex*controller->boot.blockSize + index, buffer) != 0 ) {
            fprintf(stderr, "!ERROR! // readBlockToBeingWorkedBlock // error reading sector\n");
            return ERROR;
        }
        memcpy(&beingWorkedBlock[index*SECTOR_SIZE], &buffer, sizeof(buffer));
    }
    return SUCCESS;
}

bool readBlockToAuxiliaryWorkingBlock(int blockIndex) {
    unsigned char buffer[SECTOR_SIZE];
    int index;
    for(index = 0; index < 4; index++) {
        if(read_sector(blockIndex*controller->boot.blockSize + index, buffer) != 0 ) {
            fprintf(stderr, "!ERROR! // readBlockToAuxiliaryWorkingBlock // error reading sector\n");
            return ERROR;
        }
        memcpy(&auxiliaryWorkingBlock[index*SECTOR_SIZE], &buffer, sizeof(buffer));
    }

    return SUCCESS;
}

//bool writeBeingWorkedBlockToInodesSection(int blockToBeWrittenIndex) {
//    unsigned char buffer[SECTOR_SIZE];
//    int index;
//    for(index = 0; index < 4; index++) {
//        memcpy(&buffer, &beingWorkedBlock[index*SECTOR_SIZE], sizeof(buffer));
//
//        if(write_sector(blockToBeWrittenIndex * controller->boot.blockSize + inodesStartBlock + index, buffer) != 0 )
//            return ERROR;
//    }
//
//    return SUCCESS;
//}
//
//bool writeBeingWorkedBlockToDataSection(int blockToBeWrittenIndex) {
//    unsigned char buffer[SECTOR_SIZE];
//    int index;
//    for(index = 0; index < 4; index++) {
//        memcpy(&buffer, &beingWorkedBlock[index*SECTOR_SIZE], sizeof(buffer));
//
//        if(write_sector(blockToBeWrittenIndex * controller->boot.blockSize + blocksStartBlock + index, buffer) != 0 )
//            return ERROR;
//    }
//
//    return SUCCESS;
//}


bool writeBlockToInodeDataSection(unsigned char* blockToBeWritten, int blockToBeWrittenIndex) {

    unsigned char buffer[SECTOR_SIZE];
    int index;
    for(index = 0; index < 4; index++) {
        memcpy(&buffer, &blockToBeWritten[index*SECTOR_SIZE], sizeof(buffer));

        if(write_sector(blockToBeWrittenIndex * controller->boot.blockSize + inodesStartBlock + index, buffer) != 0 ) {
            fprintf(stderr, "!ERROR! // writeBlockToInodeDataSection // error writing sector\n");
            return ERROR;
        }
    }

    return SUCCESS;
}

bool writeBlockToBlockDataSection(unsigned char* blockToBeWritten, int blockToBeWrittenIndex) {
        if(blockToBeWrittenIndex < inodesStartBlock) {
        fprintf(stderr, "!ERROR! // writeBlockToInodeDataSection // invalid arguments tried to write in boot\n");
        return ERROR;
    }

    unsigned char buffer[SECTOR_SIZE];
    int index;
    for(index = 0; index < 4; index++) {
        memcpy(&buffer, &blockToBeWritten[index*SECTOR_SIZE], sizeof(buffer));

        if(write_sector(blockToBeWrittenIndex * controller->boot.blockSize + index, buffer) != 0 ) {
            fprintf(stderr, "!ERROR! // writeBlockToBlockDataSection // error writing sector\n");
            return ERROR;
            }
    }

    return SUCCESS;
}

bool readAdressesToDataPointerCollection (DWORD* collection, DWORD adress, int amountOfDesiredAdresses) {
    if(collection == NULL) {
        fprintf(stderr, "!ERROR! // readAdressesToDataPointerCollection // collection is NULL\n");
        return ERROR;
    }

    if(adress <= 0) {
        fprintf(stderr, "!ERROR! // readAdressesToDataPointerCollection // adress is NULL\n");
        return ERROR;
    }

    if(amountOfDesiredAdresses <= 0) {
        fprintf(stderr, "!ERROR! // readAdressesToDataPointerCollection // desiredAdresses is less or equal to zero\n");
        return ERROR;
    }

    if(amountOfDesiredAdresses > 256) {
        amountOfDesiredAdresses = 256;
    }

    readBlockToBeingWorkedBlock(adress);

    int sizeOfDWORD = sizeof(DWORD);

    int index;
    for(index = 0; index < amountOfDesiredAdresses; index++) {
        memcpy(&collection[index],&beingWorkedBlock[index*sizeOfDWORD],sizeof(DWORD));
    }

    return SUCCESS;
}
