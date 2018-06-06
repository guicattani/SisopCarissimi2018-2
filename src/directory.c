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

//TODO documentação

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

struct t2fs_record* relativePathExists(char* path, struct t2fs_record* directory) {
    if(directory->TypeVal != TYPEVAL_DIRETORIO || directory == NULL)
        return NULL;

    if(!strcmp(path, ".") || !strcmp(path, "..") || !strcmp(path, "./") || !strcmp(path, "../") || !strcmp(path, "/") || strlen(path) == 0)
        return directory;

    int index;
    struct t2fs_record* recordOfPath = NULL;
    struct t2fs_record* records;

    char* position;

    char dirBeingLookedFor[MAX_PATH_LENGTH];
    char restOfThePath[MAX_PATH_LENGTH];
    char cleanedPath[MAX_PATH_LENGTH];

    dirBeingLookedFor[0] = '\0';
    restOfThePath[0] = '\0';
    cleanedPath[0] = '\0';
    strcpy(cleanedPath,path);

    getInodeToBeingWorkedInode(directory->inodeNumber);
    DWORD adress = beingWorkedInode->dataPtr[0];

    position = strstr(cleanedPath, "/");
    if(position) {
        subString(cleanedPath, dirBeingLookedFor, 0, (int) (position - cleanedPath) );
        subString(cleanedPath, restOfThePath, (int) (position - cleanedPath) + 1, (int)strlen(cleanedPath) - (int) (position - cleanedPath) -1);
    }
    else
        strcpy(dirBeingLookedFor, cleanedPath);

    records = inodeDataPointerToRecords(adress);
    for(index = 0; index < 16; index++) {
        if(records[index].TypeVal == TYPEVAL_DIRETORIO && records[index].inodeNumber != INVALID_PTR ) {
                if(!strcmp(dirBeingLookedFor, records[index].name))
                    recordOfPath = relativePathExists(restOfThePath, &records[index]);
        }
    }
    return recordOfPath;
}

struct t2fs_record* findRecordOfPath(char* path) {
    char beingWorkedPath[MAX_PATH_LENGTH];
    assureThatPathIsAbsolute(path, beingWorkedPath);

    return relativePathExists(beingWorkedPath,rootDirectory);
}

void getPathToDirectory(struct t2fs_record* directory, char* completePath, int upperDirectoryInode) {
    if(directory == NULL)
        return;
    struct t2fs_record* records;
    int index;

    getInodeToBeingWorkedInode(directory->inodeNumber);
    records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[0]);

    if(records[0].inodeNumber == records[1].inodeNumber) {
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

struct t2fs_record* returnRecordOfParentDirectory(char* fullAbsolutePath) {
    char beingWorkedPath[MAX_PATH_LENGTH];
    assureThatPathIsAbsolute(fullAbsolutePath, beingWorkedPath);

    char* position;
    char cleanedPath[MAX_PATH_LENGTH];

    position = rstrstr(beingWorkedPath, "/");
    if(position == NULL)
        return NULL;

    subString(beingWorkedPath, cleanedPath, 0, (int) (position - beingWorkedPath) );

    struct t2fs_record* recordOfPath;
    recordOfPath = findRecordOfPath(cleanedPath);

    if(recordOfPath == NULL)
        return NULL;

    return recordOfPath;
}

void assureThatPathIsAbsolute(char* path, char* completePath) {
    if(path == NULL)
        return;

    if(path[0] == '/') {
        strcpy(completePath, &path[1]);
        return;
    }

    char* position = strstr(path, "../");
    if(position) {
        struct t2fs_record* records;
        records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[0]);
        getPathToDirectory(&records[1], completePath, -1);
        if(completePath == NULL)
            return;

        strcat(completePath,&path[2]);
    }
    else {
        position = strstr(path, "./");
        if(position) {
            struct t2fs_record* records;
            records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[0]);
            getPathToDirectory(&records[0], completePath, -1);
            if(completePath == NULL)
                return;

            strcat(completePath,&path[1]);
        }
    }

}

