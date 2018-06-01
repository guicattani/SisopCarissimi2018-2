#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../include/inodehandler.h"
#include "../include/blockhandler.h"
#include "../include/directory.h"
#include "../include/stringfunctions.h"

int accessedDirectories[256];
int counterForAccessedDirectories;

void printDirectoryTree() {
    printf(".\n");
    accessedDirectories[0] = 0;
    counterForAccessedDirectories = 1;

    printerOfTree(1, 0);
}

void printerOfTree(int level, int inodeNumber) {

    int index;
    getInodeToBeingWorkedInode(inodeNumber);

    if(beingWorkedInode->dataPtr[0] != INVALID_PTR )
        printSubDirectories(level ,beingWorkedInode->dataPtr[0]);

    if(beingWorkedInode->blocksFileSize > 1 && beingWorkedInode->dataPtr[1] != INVALID_PTR)
        printSubDirectories(level, beingWorkedInode->dataPtr[1]);

    if(beingWorkedInode->blocksFileSize > 2 && beingWorkedInode->singleIndPtr != INVALID_PTR ) {
        readBlockToAuxiliaryWorkingBlock(beingWorkedInode->singleIndPtr);
        for(index = 0; index < 1024/sizeof(DWORD); index++ ) {
            printSubDirectories(level, auxiliaryWorkingBlock[index*sizeof(DWORD)]);
        }
    }
    if(beingWorkedInode->blocksFileSize > 256 && beingWorkedInode->doubleIndPtr != INVALID_PTR ) {
        printf("doubleIndPtr achado, tem que tratar isso\n"); //TODO
    }


}

void printSubDirectories(int level, DWORD dataPointer) {
    struct t2fs_record* records;
    records = inodeDataPointerToRecords(dataPointer);

    int index, indexTabs;

    for(index = 0; index < 16; index++) {
        if(!strcmp(records[index].name, ".") || !strcmp(records[index].name, ".."))
           continue;

        if(records[index].TypeVal == TYPEVAL_DIRETORIO && records[index].inodeNumber != INVALID_PTR ) {
            for(indexTabs = 0; indexTabs < level; indexTabs++)
                putchar('\t');

            printf("\\%s\n", records[index].name);

            for(indexTabs = 0; indexTabs < counterForAccessedDirectories; indexTabs++) {
                if (records[index].inodeNumber == accessedDirectories[indexTabs])
                    continue; //already been expanded
                }

                accessedDirectories[counterForAccessedDirectories++] = records[index].inodeNumber;
                printerOfTree(++level, records[index].inodeNumber);
                level--;
            }
    }

    free(records);
}

bool relativePathExists(char* path, struct t2fs_record* directory) {
    if(directory->TypeVal != TYPEVAL_DIRETORIO || directory == NULL)
        return false;

    if(!strcmp(path, ".") || !strcmp(path, "..") || !strcmp(path, "./") || !strcmp(path, "../") || !strcmp(path, "/") || strlen(path) == 0)
        return true;

    bool found = false;
    int index;

    struct t2fs_record* records;

    char* position;

    char dirBeingLookedFor[59];
    char restOfThePath[59];
    char cleanPath[59];

    dirBeingLookedFor[0] = '\0';
    restOfThePath[0] = '\0';
    cleanPath[0] = '\0';
    strcpy(cleanPath,path);

    getInodeToBeingWorkedInode(directory->inodeNumber);
    DWORD adress = beingWorkedInode->dataPtr[0];

    position = strstr(cleanPath, "../");
    if(position) {
        subString(path, cleanPath, 3, (int) strlen(cleanPath) - 2);

        struct t2fs_record* records;
        records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[1]);

        return relativePathExists(cleanPath,&records[1]); //search in upper folder
    }

    position = strstr(cleanPath, "./");
    if(position) {
        subString(path, cleanPath, 2, (int) strlen(cleanPath) - 2);
    }

    position = strstr(cleanPath, "/");
    if(position) {
        subString(cleanPath, dirBeingLookedFor, 0, (int) (position - cleanPath) );
        subString(cleanPath, restOfThePath, (int) (position - cleanPath) + 1, (int)strlen(cleanPath) - (int) (position - cleanPath) -1);
    }
    else
        strcpy(dirBeingLookedFor, cleanPath);

    records = inodeDataPointerToRecords(adress);
    for(index = 0; index < 16; index++) {
        if(records[index].TypeVal == TYPEVAL_DIRETORIO && records[index].inodeNumber != INVALID_PTR ) {
                if(!strcmp(dirBeingLookedFor, records[index].name))
                    found = relativePathExists(restOfThePath, &records[index]);
        }
    }
    return found;
}

bool absolutePathExists(char* path) {
    struct t2fs_record* record;
    record = inodeDataPointerGetFirstRecord(rootDirectory->inodeNumber);

    bool status = relativePathExists(path , record);
    return status;
}

void getPathToDirectory(struct t2fs_record* directory, char* completePath, int upperDirectoryInode) {
    if(directory == NULL)
        return;
    struct t2fs_record* records;
    int index;

    getInodeToBeingWorkedInode(directory->inodeNumber);
    records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[0]);

    if(records[0].inodeNumber == records[1].inodeNumber){
        completePath[0] = '/';
        completePath[1] = '\0';
    }
    else {
        getPathToDirectory(&records[1], completePath, directory->inodeNumber);

        if(upperDirectoryInode == -1) {
            completePath[strlen(completePath)-1] = '\0';
            return;
        }

        }

    for(index = 0; index < 16; index++) {
        if(records[index].TypeVal == TYPEVAL_DIRETORIO && records[index].inodeNumber != INVALID_PTR
        && upperDirectoryInode == records[index].inodeNumber) {
            strncpy(completePath + strlen(completePath), records[index].name, strlen(records[index].name));
                break;
        }
    }

    if(strlen(completePath) != 1) {
        completePath[strlen(completePath)] = '/';
        completePath[strlen(completePath)+1] = '\0';
    }

}


