#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

#include "../include/initializer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

t_control* controller;

t_control* initializeLibrary() {
    controller = (t_control*) malloc(sizeof(t_control));

    bool errorCode = bootFileSystemController();
    if(errorCode == ERROR)
        return NULL;

    errorCode = fillBitmaps();
    if(errorCode == ERROR)
        return NULL;

    return controller;
}

bool bootFileSystemController()
{

    unsigned char buffer[256];
    int index = 0;

    if(read_sector(0, buffer) != 0) {
        fprintf(stderr, "Falha ao ler setor do disco.\n");
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

    printf("Boot Data:\n");
    printf("\tid:%c%c%c%c\n\tversion:%u\n\tsuperblockSize1:%u\n\tfreeBlocksBitmapSize :%u\n\tfreeInodeBitmapSize:%u\n\tinodeAreaSize:%u\n\tblockSize:%u\n\tdiskSize:%u\n",
           controller->boot.id[0],
           controller->boot.id[1],
           controller->boot.id[2],
           controller->boot.id[3],
           controller->boot.version,
           controller->boot.superblockSize,
           controller->boot.freeBlocksBitmapSize,
           controller->boot.freeInodeBitmapSize,
           controller->boot.inodeAreaSize,
           controller->boot.blockSize,
           controller->boot.diskSize);

    if(controller->boot.version != 32289) {
        fprintf(stderr, "File system version is outdated.\n");
        return ERROR;
    }

    if(controller->boot.id[0] != 'T' || controller->boot.id[1] != '2' || controller->boot.id[2] != 'F' || controller->boot.id[3] != 'S') {
        fprintf(stderr, "File system id is invalid.\n");
        return ERROR;
    }

    return SUCCESS;
}

bool fillBitmaps() {

    if(controller == NULL)
        return ERROR;

    unsigned char buffer[256];

    if(read_sector(1, buffer) != 0) {
        fprintf(stderr, "Falha ao ler setor do disco.\n");
        return ERROR;
    }

    controller->freeBlockBitmap = (int) buffer;

    if(read_sector(1, buffer) != 0) {
        fprintf(stderr, "Falha ao ler setor do disco.\n");
        return ERROR;
    }

    controller->freeInodeBitmap = (int) buffer;

    return SUCCESS;
}
