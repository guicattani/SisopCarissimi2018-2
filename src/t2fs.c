#include "../include/t2fs.h"
#include "../include/initializer.h"
#include "../include/directory.h"
#include "../include/inodehandler.h"
#include "../include/blockhandler.h"
#include "../include/stringfunctions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool initialized = false;

/**
T2FS (volume) está dividido em cinco áreas:
    superbloco
    bitmap de blocos do disco livres e ocupados
    bitmap de i-nodes livres e ocupados,
    i-nodes
    blocos de dadosĶ

Um volume T2FS é formado por S setores (0 a S-1)
    onde os primeiros setores armazenam as estruturas gerenciais que definem e
    controlam a cartografia do disco T2FS, e os setores restantes são agrupados,
    n a n, formando um bloco de dados

    Portanto, os blocos T2FS são formados por n setores contíguos e são numerados de 0 a B-1,
    onde B é o número total de blocos no disco. Esses valores são fornecidos no superbloco
    (ver tabela 1 na especificação)

**/

/*-----------------------------------------------------------------------------
Função: Usada para identificar os desenvolvedores do T2FS.
	Essa função copia um string de identificação para o ponteiro indicado por "name".
	Essa cópia não pode exceder o tamanho do buffer, informado pelo parâmetro "size".
	O string deve ser formado apenas por caracteres ASCII (Valores entre 0x20 e 0x7A) e terminado por ‘\0’.
	O string deve conter o nome e número do cartão dos participantes do grupo.

Entra:	name -> buffer onde colocar o string de identificação.
	size -> tamanho do buffer "name" (número máximo de bytes a serem copiados).

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int identify2 (char *name, int size) {
   if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // identify2 // failed initializing\n");
        return ERROR;
    }

    if(size < SIZEIDENTIFY) {
        fprintf(stderr, "!ERROR! // identify2 // argument 'size' is less than string total length\n");
        return ERROR;
    }


    int index = 0;
    while ((name[index] = GROUPNAMESTRING[index]) != '\0') {
        index++;
    }

    return SUCCESS;
}


/*-----------------------------------------------------------------------------
Função: Criar um novo arquivo.
	O nome desse novo arquivo é aquele informado pelo parâmetro "filename".
	O contador de posição do arquivo (current pointer) deve ser colocado na posição zero.
	Caso já exista um arquivo ou diretório com o mesmo nome, a função deverá retornar um erro de criação.
	A função deve retornar o identificador (handle) do arquivo.
	Esse handle será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do arquivo criado.

Entra:	filename -> nome do arquivo a ser criado.

Saída:	Se a operação foi realizada com sucesso, a função retorna o handle do arquivo (número positivo).
	Em caso de erro, deve ser retornado um valor negativo.
-----------------------------------------------------------------------------*/
FILE2 create2 (char *filename) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // create2 // failed initializing\n");
        return ERROR;
    }

    if(filename == NULL) {
        fprintf(stderr, "!ERROR! // create2 // filename is NULL\n");
        return ERROR;
    }

    if(filename[0] == '.' && strlen(filename) == 1) {
        fprintf(stderr, "!ERROR! // create2 // reserved character\n");
        return ERROR;
    }

    if(filename[0] == '.' && filename[1] == '.' && strlen(filename) == 2) {
        fprintf(stderr, "!ERROR! // create2 // reserved character\n");
        return ERROR;
    }

    int inodeNumber = 0;

    char cleanedPath[MAX_FILE_NAME_SIZE];
    char nameOfFile[MAX_FILE_NAME_SIZE];


    char* position = rstrstr(filename, "/");
    if(position == NULL) {
        memcpy(nameOfFile, filename, strlen(filename)+1);
        memcpy(cleanedPath, filename, strlen(filename)+1);
    }
    else if(position == filename) {
        memcpy(nameOfFile, filename+1, strlen(filename));
        memcpy(cleanedPath, filename+1, strlen(filename));
    }
    else{
        subString(filename, cleanedPath, 0, (int) (position - filename) );
        subString(filename, nameOfFile, (int) (position - filename) + 1, (int)strlen(filename) - (int) (position - filename) -1);
    }

    struct t2fs_record* record = returnRecordOfParentDirectory(filename);

    if(record != NULL) {
        inodeNumber = record->inodeNumber;
    }

    position = strstr(cleanedPath, "../");
    if(position) {
//        getInodeToBeingWorkedInode(currentDirectory->inodeNumber);
//        struct t2fs_record* records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[0]);
//        inodeNumber = records[1].inodeNumber;

        subString(filename, cleanedPath, 3, (int) strlen(cleanedPath) - 3);
    }

    position = strstr(cleanedPath, "..");
    if(position) {
//        getInodeToBeingWorkedInode(currentDirectory->inodeNumber);
//        struct t2fs_record* records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[0]);
//        inodeNumber = records[1].inodeNumber;

        subString(filename, cleanedPath, 2, (int) strlen(cleanedPath) - 2);
    }

    position = strstr(cleanedPath, "./");
    if(position) {
        subString(filename, cleanedPath, 2, (int) strlen(cleanedPath) - 2);
    }

    position = strstr(cleanedPath, ".");
    if(position) {
        subString(filename, cleanedPath, 2, (int) strlen(cleanedPath) - 2);
    }

    if(filename[0] == '/') {
        inodeNumber = rootDirectory->inodeNumber;
    }

    getInodeToBeingWorkedInode(inodeNumber);

    int firstAdress = beingWorkedInode->dataPtr[0];
    int secondAdress = beingWorkedInode->dataPtr[1];
//    int singleIndirectPointer = beingWorkedInode->singleIndPtr;
//    int doubleIndirectPointer = beingWorkedInode->doubleIndPtr;

    int handle = searchVacantSpaceAndCreateFile(firstAdress, nameOfFile, filename);
    if(secondAdress != INVALID_PTR && handle < 0)
        handle = searchVacantSpaceAndCreateFile(secondAdress, nameOfFile, filename);
    if(handle < 0)  {
        fprintf(stderr, "!ERROR! // create2 // failed creating file not enough space in records\n");
        return ERROR;
    }

    return handle;
}


/*-----------------------------------------------------------------------------
Função:	Apagar um arquivo do disco.
	O nome do arquivo a ser apagado é aquele informado pelo parâmetro "filename".

Entra:	filename -> nome do arquivo a ser apagado.

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int delete2 (char *filename) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // delete2 // failed initializing\n");
        return ERROR;
    }

    if(filename == NULL) {
        fprintf(stderr, "!ERROR! // delete2 // filename is NULL\n");
        return ERROR;
    }
    int index;
    for(index = 0; index < MAX_OPEN_FILES; index++) {
        if(openedFiles[index].valid && !strcmp(filename, openedFiles[index].fileRecord->name)) {
            fprintf(stderr, "!WARNING! // delete2 // file opened\n");
            close2(index);
            break;
        }

    }

    char nameOfFile[MAX_FILE_NAME_SIZE];

    char* position = rstrstr(filename, "/");
    if(position == NULL) {
        memcpy(nameOfFile, filename, strlen(filename)+1);
    }
    else if(position == filename) {
        memcpy(nameOfFile, filename+1, strlen(filename));
    }
    else{
        subString(filename, nameOfFile, (int) (position - filename) + 1, (int)strlen(filename) - (int) (position - filename) -1);
    }

    char pathOfDir[MAX_FILE_NAME_SIZE];
    getNameOfDirectoryAtEndOfPath(filename,pathOfDir);

    struct t2fs_record* record = findRecordOfPath(pathOfDir);
    getInodeToBeingWorkedInode(record->inodeNumber);

    int firstAdress = beingWorkedInode->dataPtr[0];;
    int secondAdress = beingWorkedInode->dataPtr[1];
//    int singleIndirectPointer = beingWorkedInode->singleIndPtr;
//    int doubleIndirectPointer = beingWorkedInode->doubleIndPtr;

    if(firstAdress != INVALID_PTR && !searchRecordAndRemoveFile(firstAdress, nameOfFile))
        return SUCCESS;
    if(secondAdress != INVALID_PTR && !searchRecordAndRemoveFile(secondAdress, nameOfFile))
        return SUCCESS;

    fprintf(stderr, "!ERROR! // delete2 // failed deleting file\n");
    return ERROR;
}


/*-----------------------------------------------------------------------------
Função:	Abre um arquivo existente no disco.
	O nome desse novo arquivo é aquele informado pelo parâmetro "filename".
	Ao abrir um arquivo, o contador de posição do arquivo (current pointer) deve ser colocado na posição zero.
	A função deve retornar o identificador (handle) do arquivo.
	Esse handle será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do arquivo criado.
	Todos os arquivos abertos por esta chamada são abertos em leitura e em escrita.
	O ponto em que a leitura, ou escrita, será realizada é fornecido pelo valor current_pointer (ver função seek2).

Entra:	filename -> nome do arquivo a ser apagado.

Saída:	Se a operação foi realizada com sucesso, a função retorna o handle do arquivo (número positivo)
	Em caso de erro, deve ser retornado um valor negativo
-----------------------------------------------------------------------------*/
FILE2 open2 (char *filename) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // open2 // failed initializing\n");
        return ERROR;
    }
    if(filename == NULL) {
        fprintf(stderr, "!ERROR! // open2 // filename is NULL\n");
        return ERROR;
    }

    int index;
    int emptySpace = -1;
    for(index = 0; index < MAX_OPEN_FILES; index++) {
        if(openedFiles[index].valid == true) {
            if(openedFiles[index].valid && !strcmp(filename, openedFiles[index].fileRecord->name)) {
                fprintf(stderr, "!ERROR! // open2 // file already opened\n");
                return ERROR;
            }
        }
        else if(emptySpace == -1)
            emptySpace = index;
    }

    struct t2fs_record* recordOfFile = NULL;
    recordOfFile = findRecordOfFile(filename);

    if(recordOfFile == NULL) {
        fprintf(stderr, "!ERROR! // open2 // failed to find record of file\n");
        return ERROR;
    }

    struct openFile* newOpenedFile = malloc(sizeof(struct openFile));

    newOpenedFile->valid = true;
    newOpenedFile->currentPointer = 0;

    newOpenedFile->fileInode = malloc(sizeof(struct t2fs_inode));
    newOpenedFile->fileRecord = malloc(sizeof(struct t2fs_record));

    getInodeToBeingWorkedInode(recordOfFile->inodeNumber);
    *newOpenedFile->fileInode = *beingWorkedInode;
    *newOpenedFile->fileRecord = *recordOfFile;

    openedFiles[emptySpace] = *newOpenedFile;

    free(newOpenedFile);

    return emptySpace;
}


/*-----------------------------------------------------------------------------
Função:	Fecha o arquivo identificado pelo parâmetro "handle".

Entra:	handle -> identificador do arquivo a ser fechado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // close2 // failed initializing\n");
        return ERROR;
    }

    if(!isFileHandleValid(handle)) {
        fprintf(stderr, "!ERROR! // close2 // handle is invalid\n");
        return ERROR;
    }

    openedFiles[handle].valid = false;

    return SUCCESS;
}


/*-----------------------------------------------------------------------------
Função:	Realiza a leitura de "size" bytes do arquivo identificado por "handle".
	Os bytes lidos são colocados na área apontada por "buffer".
	Após a leitura, o contador de posição (current pointer) deve ser ajustado para o byte seguinte ao último lido.

Entra:	handle -> identificador do arquivo a ser lido
	buffer -> buffer onde colocar os bytes lidos do arquivo
	size -> número de bytes a serem lidos

Saída:	Se a operação foi realizada com sucesso, a função retorna o número de bytes lidos.
	Se o valor retornado for menor do que "size", então o contador de posição atingiu o final do arquivo.
	Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
int read2 (FILE2 handle, char *buffer, int size) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // read2 // failed initializing\n");
        return ERROR;
    }

    if(!isFileHandleValid(handle)) {
        fprintf(stderr, "!ERROR! // read2 // handle is invalid\n");
        return ERROR;
    }

    if(size <= 0) {
        fprintf(stderr, "!ERROR! // read2 // argument 'size' is negative\n");
        return ERROR;
    }

    if(size > openedFiles[handle].fileInode->bytesFileSize) {
        fprintf(stderr, "!WARNING! // read2 // argument 'size' is bigger than filesize, adjusting\n");
        size = openedFiles[handle].fileInode->bytesFileSize;
    }

    int bytesRead = 0;

    char blockBuffer[1024];
    char bufferOfBuffer[1024];
    int currentPositionOfBuffer = 0;

    int bytesToBeReadFromCurrentBlock;
    int bytesLeftInCurrentBlock = 1024 - openedFiles[handle].currentPointer % 1024;

    if(size - bytesLeftInCurrentBlock > 0)
        bytesToBeReadFromCurrentBlock = bytesLeftInCurrentBlock;
    else
        bytesToBeReadFromCurrentBlock = size;

    int completeBlocksToBeRead =  (int)((size - bytesLeftInCurrentBlock)/1024);
    int bytesRemainderOfBlocksToBeRead = size - completeBlocksToBeRead*1024 - bytesToBeReadFromCurrentBlock;

    if(openedFiles[handle].currentBlock == 0) {
        readBlockToBuffer(openedFiles[handle].fileInode->dataPtr[0],blockBuffer);
        memcpy(&bufferOfBuffer, &blockBuffer[openedFiles[handle].currentPointer], bytesToBeReadFromCurrentBlock);
        memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeReadFromCurrentBlock);
        currentPositionOfBuffer += bytesToBeReadFromCurrentBlock;

        openedFiles[handle].currentPointer += bytesToBeReadFromCurrentBlock;
        bytesRead += bytesToBeReadFromCurrentBlock;
        openedFiles[handle].currentBlock = (int) (openedFiles[handle].currentPointer / 1024);
    }

    if(openedFiles[handle].currentBlock == 1) {

        if(size - currentPositionOfBuffer > 1024)
            bytesToBeReadFromCurrentBlock = 1024;
        else
            bytesToBeReadFromCurrentBlock = bytesRemainderOfBlocksToBeRead;

        readBlockToBuffer(openedFiles[handle].fileInode->dataPtr[1],blockBuffer);
        memcpy(&bufferOfBuffer, &blockBuffer[openedFiles[handle].currentPointer], bytesToBeReadFromCurrentBlock);
        memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeReadFromCurrentBlock);
        currentPositionOfBuffer += bytesToBeReadFromCurrentBlock;

        openedFiles[handle].currentPointer += bytesToBeReadFromCurrentBlock;
        bytesRead += bytesToBeReadFromCurrentBlock;
        openedFiles[handle].currentBlock = (int) (openedFiles[handle].currentPointer / 1024);
    }

    if(openedFiles[handle].currentBlock > 1 && openedFiles[handle].currentBlock < 258) { //256 indirects 2 directs
        int adressesPerPointerBlock = 1024/sizeof(DWORD);
        DWORD collectionOfDataPointers[adressesPerPointerBlock];
        if(bytesRemainderOfBlocksToBeRead>0)
            readAdressesToDataPointerCollection(collectionOfDataPointers, openedFiles[handle].fileInode->singleIndPtr, completeBlocksToBeRead+1); //if there is a remainder
        else
            readAdressesToDataPointerCollection(collectionOfDataPointers, openedFiles[handle].fileInode->singleIndPtr, completeBlocksToBeRead);
        int index = 0;
        while(currentPositionOfBuffer < size) {
            if(size - currentPositionOfBuffer > 1024)
                bytesToBeReadFromCurrentBlock = 1024;
            else
                bytesToBeReadFromCurrentBlock = bytesRemainderOfBlocksToBeRead;

            readBlockToBuffer(collectionOfDataPointers[index], blockBuffer);
            memcpy(&bufferOfBuffer, &blockBuffer[openedFiles[handle].currentPointer], bytesToBeReadFromCurrentBlock);
            memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeReadFromCurrentBlock);

            currentPositionOfBuffer += bytesToBeReadFromCurrentBlock;
            bytesRead += bytesToBeReadFromCurrentBlock;

            openedFiles[handle].currentPointer += bytesToBeReadFromCurrentBlock;
            openedFiles[handle].currentBlock = (int) (openedFiles[handle].currentPointer / 1024);

            index++;
            completeBlocksToBeRead--;
        }
    }
    if(openedFiles[handle].currentBlock > 258 && currentPositionOfBuffer < size) { //256 indirects 2 directs
        int adressesPerPointerBlock = 1024/sizeof(DWORD);
        DWORD collectionOfDataPointers[adressesPerPointerBlock];
        DWORD auxiliaryCollectionOfDataPointers[adressesPerPointerBlock];
        readAdressesToDataPointerCollection(collectionOfDataPointers,openedFiles[handle].fileInode->doubleIndPtr, completeBlocksToBeRead+1);

        int index = 0;
        while(currentPositionOfBuffer < size ) {
            int secondIndex = 0;
            readAdressesToDataPointerCollection(auxiliaryCollectionOfDataPointers,collectionOfDataPointers[index], completeBlocksToBeRead+1);
            while(currentPositionOfBuffer < size || secondIndex < adressesPerPointerBlock) {
                if(size - currentPositionOfBuffer > 1024)
                    bytesToBeReadFromCurrentBlock = 1024;
                else
                    bytesToBeReadFromCurrentBlock = bytesRemainderOfBlocksToBeRead;

                readBlockToBuffer(collectionOfDataPointers[index],blockBuffer);
                memcpy(&bufferOfBuffer, &blockBuffer[openedFiles[handle].currentPointer], bytesToBeReadFromCurrentBlock);
                memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeReadFromCurrentBlock);

                currentPositionOfBuffer += bytesToBeReadFromCurrentBlock;
                bytesRead += bytesToBeReadFromCurrentBlock;

                openedFiles[handle].currentPointer += bytesToBeReadFromCurrentBlock;
                openedFiles[handle].currentBlock = (int) (openedFiles[handle].currentPointer / 1024);

                secondIndex++;
                completeBlocksToBeRead--;
            }
            index++;
        }
    }

    return bytesRead;
}


/*-----------------------------------------------------------------------------
Função:	Realiza a escrita de "size" bytes no arquivo identificado por "handle".
	Os bytes a serem escritos estão na área apontada por "buffer".
	Após a escrita, o contador de posição (current pointer) deve ser ajustado para o byte seguinte ao último escrito.

Entra:	handle -> identificador do arquivo a ser escrito
	buffer -> buffer de onde pegar os bytes a serem escritos no arquivo
	size -> número de bytes a serem escritos

Saída:	Se a operação foi realizada com sucesso, a função retorna o número de bytes efetivamente escritos.
	Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
int write2 (FILE2 handle, char *buffer, int size) {
//    if(checkInitialization()) {
//        fprintf(stderr, "!ERROR! // write2 // failed initializing\n");
//        return ERROR;
//    }
//
//    if(!isFileHandleValid(handle)) {
//        fprintf(stderr, "!ERROR! // write2 // handle is invalid\n");
//        return ERROR;
//    }
//
//    if(size <= 0) {
//        fprintf(stderr, "!ERROR! // write2 // argument 'size' is negative\n");
//        return ERROR;
//    }
//
//    int bufferOfBuffer[1024];
//    int currentPositionOfBuffer = 0;
//
//    int bytesToBeWrittenToCurrentBlock;
//    int bytesLeftInCurrentBlock = 1024 - openedFiles[handle].currentPointer % 1024;
//
//    if(size - bytesLeftInCurrentBlock > 0)
//        bytesToBeWrittenToCurrentBlock = bytesLeftInCurrentBlock;
//    else
//        bytesToBeWrittenToCurrentBlock = size;
//
//    int completeBlocksToBeWritten=  floor((size - bytesLeftInCurrentBlock)/1024);
//    int bytesRemainderOfBlocksToBeWritten = size - completeBlocksToBeWritten*1024 - bytesToBeWrittenToCurrentBlock;
//
//    int vacantInode;
//    int vacantBlock;
//
//    if(openedFiles[handle].currentBlock == 0) {0
//        readBlockToBeingWorkedBlock(openedFiles[handle].fileInode->dataPtr[0]);
//        memcpy(&bufferOfBuffer, &beingWorkedBlock[openedFiles[handle].currentPointer], bytesToBeWrittenToCurrentBlock);
//        memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeWrittenToCurrentBlock);
//        currentPositionOfBuffer += bytesToBeWrittenToCurrentBlock;
//
//        openedFiles[handle].currentPointer += bytesToBeWrittenToCurrentBlock;
//        openedFiles[handle].currentBlock = (int) floor(openedFiles[handle].currentPointer / 1024);
//    }
//
//    if(openedFiles[handle].currentBlock == 1) {
//
//        if(size - currentPositionOfBuffer > 1024)
//            bytesToBeWrittenToCurrentBlock = 1024;
//        else
//            bytesToBeWrittenToCurrentBlock = bytesRemainderOfBlocksToBeWritten;
//
//        readBlockToBeingWorkedBlock(openedFiles[handle].fileInode->dataPtr[1]);
//        memcpy(&bufferOfBuffer, &beingWorkedBlock[openedFiles[handle].currentPointer], bytesToBeWrittenToCurrentBlock);
//        memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeWrittenToCurrentBlock);
//        currentPositionOfBuffer += bytesToBeWrittenToCurrentBlock;
//
//        openedFiles[handle].currentPointer += bytesToBeWrittenToCurrentBlock;
//        openedFiles[handle].currentBlock = (int) floor(openedFiles[handle].currentPointer / 1024);
//    }
//
//    if(openedFiles[handle].currentBlock > 1 && openedFiles[handle].currentBlock < 258) { //256 indirects 2 directs
//        int adressesPerPointerBlock = 1024/sizeof(DWORD);
//        DWORD collectionOfDataPointers[adressesPerPointerBlock];
//        if(bytesRemainderOfBlocksToBeWritten>0)
//            readAdressesToDataPointerCollection(collectionOfDataPointers, openedFiles[handle].fileInode->singleIndPtr, completeBlocksToBeWritten+1); //if there is a remainder
//        else
//            readAdressesToDataPointerCollection(collectionOfDataPointers, openedFiles[handle].fileInode->singleIndPtr, completeBlocksToBeWritten);
//        int index = 0;
//        while(currentPositionOfBuffer < size) {
//            if(size - currentPositionOfBuffer > 1024)
//                bytesToBeWrittenToCurrentBlock = 1024;
//            else
//                bytesToBeWrittenToCurrentBlock = bytesRemainderOfBlocksToBeWritten;
//
//            readBlockToBeingWorkedBlock(collectionOfDataPointers[index]);
//            memcpy(&bufferOfBuffer, &beingWorkedBlock[openedFiles[handle].currentPointer], bytesToBeWrittenToCurrentBlock);
//            memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeWrittenToCurrentBlock);
//            currentPositionOfBuffer += bytesToBeWrittenToCurrentBlock;
//
//            openedFiles[handle].currentPointer += bytesToBeWrittenToCurrentBlock;
//            openedFiles[handle].currentBlock = (int) floor(openedFiles[handle].currentPointer / 1024);
//
//            index++;
//            completeBlocksToBeWritten--;
//        }
//    }
//    if(openedFiles[handle].currentBlock > 258 && currentPositionOfBuffer < size) { //256 indirects 2 directs
//        int adressesPerPointerBlock = 1024/sizeof(DWORD);
//        DWORD collectionOfDataPointers[adressesPerPointerBlock];
//        DWORD auxiliaryCollectionOfDataPointers[adressesPerPointerBlock];
//        readAdressesToDataPointerCollection(collectionOfDataPointers,openedFiles[handle].fileInode->doubleIndPtr, completeBlocksToBeWritten+1);
//
//        int index = 0;
//        while(currentPositionOfBuffer < size ) {
//            int secondIndex = 0;
//            readAdressesToDataPointerCollection(auxiliaryCollectionOfDataPointers,collectionOfDataPointers[index], completeBlocksToBeWritten+1);
//            while(currentPositionOfBuffer < size || secondIndex < adressesPerPointerBlock) {
//                if(size - currentPositionOfBuffer > 1024)
//                    bytesToBeWrittenToCurrentBlock = 1024;
//                else
//                    bytesToBeWrittenToCurrentBlock = bytesRemainderOfBlocksToBeWritten;
//
//                readBlockToBeingWorkedBlock(collectionOfDataPointers[index]);
//                memcpy(&bufferOfBuffer, &beingWorkedBlock[openedFiles[handle].currentPointer], bytesToBeWrittenToCurrentBlock);
//                memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeWrittenToCurrentBlock);
//                currentPositionOfBuffer += bytesToBeWrittenToCurrentBlock;
//
//                openedFiles[handle].currentPointer += bytesToBeWrittenToCurrentBlock;
//                openedFiles[handle].currentBlock = (int) floor(openedFiles[handle].currentPointer / 1024);
//
//                secondIndex++;
//                completeBlocksToBeWritten--;
//            }
//            index++;
//        }
//    }
//
    return ERROR;
}


/*-----------------------------------------------------------------------------
Função:	Função usada para truncar um arquivo.
	Remove do arquivo todos os bytes a partir da posição atual do contador de posição (CP)
	Todos os bytes a partir da posição CP (inclusive) serão removidos do arquivo.
	Após a operação, o arquivo deverá contar com CP bytes e o ponteiro estará no final do arquivo

Entra:	handle -> identificador do arquivo a ser truncado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int truncate2 (FILE2 handle) {
//    if(checkInitialization()) {
//        fprintf(stderr, "!ERROR! // truncate2 // failed initializing\n");
//        return ERROR;
//    }
//
//    if(!isFileHandleValid(handle)) {
//        fprintf(stderr, "!ERROR! // truncate2 // handle is invalid\n");
//        return ERROR;
//    }
//
//    int size = openedFiles[handle].currentPointer - openedFiles[handle].fileInode->bytesFileSize;
//
//    int bufferOfBuffer[1024];
//    int currentPositionOfBuffer = 0;
//
//    int bytesToBeReadFromCurrentBlock;
//    int bytesLeftInCurrentBlock = 1024 - openedFiles[handle].currentPointer % 1024;
//
//    if(size - bytesLeftInCurrentBlock > 0)
//        bytesToBeReadFromCurrentBlock = bytesLeftInCurrentBlock;
//    else
//        bytesToBeReadFromCurrentBlock = size;
//
//    int completeBlocksToBeRead =  floor((size - bytesLeftInCurrentBlock)/1024);
//    int bytesRemainderOfBlocksToBeRead = size - completeBlocksToBeRead*1024 - bytesToBeReadFromCurrentBlock;
//
//    if(openedFiles[handle].currentBlock == 0) {
//        char bufferBlock[1024];
//        memset(bufferBlock, 0, sizeof(bufferBlock));
//        readBlockToBeingWorkedBlock(openedFiles[handle].fileInode->dataPtr[0]);
//        memcpy(bufferBlock, beingWorkedBlock, currentPointer);
//
//        writeBlockToBlockDataSection(bufferBlock,openedFiles[handle].fileInode->dataPtr[0]);
//
//        openedFiles[handle].currentPointer--; //pointer at end of file
//        openedFiles[handle].fileInode->bytesFileSize = openedFiles[handle].currentPointer;
//    }
//
//    if(openedFiles[handle].currentBlock == 1) {
//
//        if(size - currentPositionOfBuffer > 1024)
//            bytesToBeReadFromCurrentBlock = 1024;
//        else
//            bytesToBeReadFromCurrentBlock = bytesRemainderOfBlocksToBeRead;
//
//        readBlockToBeingWorkedBlock(openedFiles[handle].fileInode->dataPtr[1]);
//        memcpy(&bufferOfBuffer, &beingWorkedBlock[openedFiles[handle].currentPointer], bytesToBeReadFromCurrentBlock);
//        memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeReadFromCurrentBlock);
//        currentPositionOfBuffer += bytesToBeReadFromCurrentBlock;
//
//        openedFiles[handle].currentPointer += bytesToBeReadFromCurrentBlock;
//        openedFiles[handle].currentBlock = (int) floor(openedFiles[handle].currentPointer / 1024);
//    }
//
//    if(openedFiles[handle].currentBlock > 1 && openedFiles[handle].currentBlock < 258) { //256 indirects 2 directs
//        int adressesPerPointerBlock = 1024/sizeof(DWORD);
//        DWORD collectionOfDataPointers[adressesPerPointerBlock];
//        if(bytesRemainderOfBlocksToBeRead>0)
//            readAdressesToDataPointerCollection(collectionOfDataPointers, openedFiles[handle].fileInode->singleIndPtr, completeBlocksToBeRead+1); //if there is a remainder
//        else
//            readAdressesToDataPointerCollection(collectionOfDataPointers, openedFiles[handle].fileInode->singleIndPtr, completeBlocksToBeRead);
//        int index = 0;
//        while(currentPositionOfBuffer < size) {
//            if(size - currentPositionOfBuffer > 1024)
//                bytesToBeReadFromCurrentBlock = 1024;
//            else
//                bytesToBeReadFromCurrentBlock = bytesRemainderOfBlocksToBeRead;
//
//            readBlockToBeingWorkedBlock(collectionOfDataPointers[index]);
//            memcpy(&bufferOfBuffer, &beingWorkedBlock[openedFiles[handle].currentPointer], bytesToBeReadFromCurrentBlock);
//            memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeReadFromCurrentBlock);
//            currentPositionOfBuffer += bytesToBeReadFromCurrentBlock;
//
//            openedFiles[handle].currentPointer += bytesToBeReadFromCurrentBlock;
//            openedFiles[handle].currentBlock = (int) floor(openedFiles[handle].currentPointer / 1024);
//
//            index++;
//            completeBlocksToBeRead--;
//        }
//    }
//    if(openedFiles[handle].currentBlock > 258 && currentPositionOfBuffer < size) { //256 indirects 2 directs
//        int adressesPerPointerBlock = 1024/sizeof(DWORD);
//        DWORD collectionOfDataPointers[adressesPerPointerBlock];
//        DWORD auxiliaryCollectionOfDataPointers[adressesPerPointerBlock];
//        readAdressesToDataPointerCollection(collectionOfDataPointers,openedFiles[handle].fileInode->doubleIndPtr, completeBlocksToBeRead+1);
//
//        int index = 0;
//        while(currentPositionOfBuffer < size ) {
//            int secondIndex = 0;
//            readAdressesToDataPointerCollection(auxiliaryCollectionOfDataPointers,collectionOfDataPointers[index], completeBlocksToBeRead+1);
//            while(currentPositionOfBuffer < size || secondIndex < adressesPerPointerBlock) {
//                if(size - currentPositionOfBuffer > 1024)
//                    bytesToBeReadFromCurrentBlock = 1024;
//                else
//                    bytesToBeReadFromCurrentBlock = bytesRemainderOfBlocksToBeRead;
//
//                readBlockToBeingWorkedBlock(collectionOfDataPointers[index]);
//                memcpy(&bufferOfBuffer, &beingWorkedBlock[openedFiles[handle].currentPointer], bytesToBeReadFromCurrentBlock);
//                memcpy(&buffer[currentPositionOfBuffer], &bufferOfBuffer, bytesToBeReadFromCurrentBlock);
//                currentPositionOfBuffer += bytesToBeReadFromCurrentBlock;
//
//                openedFiles[handle].currentPointer += bytesToBeReadFromCurrentBlock;
//                openedFiles[handle].currentBlock = (int) floor(openedFiles[handle].currentPointer / 1024);
//
//                secondIndex++;
//                completeBlocksToBeRead--;
//            }
//            index++;
//        }
//    }

    return ERROR;
}


/*-----------------------------------------------------------------------------
Função:	Reposiciona o contador de posições (current pointer) do arquivo identificado por "handle".
	A nova posição é determinada pelo parâmetro "offset".
	O parâmetro "offset" corresponde ao deslocamento, em bytes, contados a partir do início do arquivo.
	Se o valor de "offset" for "-1", o current_pointer deverá ser posicionado no byte seguinte ao final do arquivo,
		Isso é útil para permitir que novos dados sejam adicionados no final de um arquivo já existente.

Entra:	handle -> identificador do arquivo a ser escrito
	offset -> deslocamento, em bytes, onde posicionar o "current pointer".

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int seek2 (FILE2 handle, DWORD offset) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // seek2 // failed initializing\n");
        return ERROR;
    }

    if(!isFileHandleValid(handle))
    {
        fprintf(stderr, "!ERROR! // seek2 // file handle invalid\n");
        return ERROR;
    }

    if (offset == (DWORD) -1)
        openedFiles[handle].currentPointer = openedFiles[handle].fileInode->bytesFileSize - 1;
    else
        openedFiles[handle].currentPointer = offset;

    return SUCCESS;
}


/*-----------------------------------------------------------------------------
Função:	Criar um novo diretório.
	O caminho desse novo diretório é aquele informado pelo parâmetro "pathname".
		O caminho pode ser ser absoluto ou relativo.
	São considerados erros de criação quaisquer situações em que o diretório não possa ser criado.
		Isso inclui a existência de um arquivo ou diretório com o mesmo "pathname".

Entra:	pathname -> caminho do diretório a ser criado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int mkdir2 (char *pathname) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // mkdir2 // failed initializing\n");
        return ERROR;
    }
    if(pathname == NULL) {
        fprintf(stderr, "!ERROR! // mkdir2 // pathname NULL\n");
        return ERROR;
    }

    char *position = rstrstr(pathname, "/");
    if(position != NULL) {
        struct t2fs_record* recordOfParentDirectory;
        recordOfParentDirectory = returnRecordOfParentDirectory(pathname);

        if(recordOfParentDirectory == NULL) {
            fprintf(stderr, "!ERROR! // mkdir2 // record of parent directory not found\n");
            return ERROR;
        }

        if(!makeRecordsForNewDir(recordOfParentDirectory, pathname))
            return SUCCESS;
    }

    else {
        if(!makeRecordsForNewDir(currentDirectory, pathname))
                return SUCCESS;
    }

    return ERROR;
}


/*-----------------------------------------------------------------------------
Função:	Apagar um subdiretório do disco.
	O caminho do diretório a ser apagado é aquele informado pelo parâmetro "pathname".
	São considerados erros quaisquer situações que impeçam a operação.
		Isso inclui:
			(a) o diretório a ser removido não está vazio;
			(b) "pathname" não existente;
			(c) algum dos componentes do "pathname" não existe (caminho inválido);
			(d) o "pathname" indicado não é um arquivo;

Entra:	pathname -> caminho do diretório a ser criado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int rmdir2 (char *pathname) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // rmdir2 // failed initializing\n");
        return ERROR;
    }

    if(pathname == NULL) {
        fprintf(stderr, "!ERROR! // rmdir2 // pathname is NULL\n");
        return ERROR;
    }

    int index;

    char nameOfDir[MAX_FILE_NAME_SIZE];
    char nameOfDirRecord[MAX_FILE_NAME_SIZE];
    getNameOfDirectoryAtEndOfPath(pathname, nameOfDir);


    for(index = 0; index < MAX_OPEN_DIRECTORIES; index++) {
        if(openedDirectories[index].valid) {
            getNameOfFileByInode(openedDirectories[index].directoryRecord->inodeNumber, nameOfDirRecord);
            if(openedDirectories[index].valid && !strcmp(nameOfDir, nameOfDirRecord)) {
                fprintf(stderr, "!WARNING! // rmdir2 // dir opened\n");
                closedir2(index);
                break;
            }
        }
    }

    struct t2fs_record* recordOfPath;
    recordOfPath = findRecordOfPath(pathname);

    if(recordOfPath == NULL) {
        fprintf(stderr, "!ERROR! // rmdir2 // record of path not found\n");
        return ERROR;
    }

    getInodeToBeingWorkedInode(recordOfPath->inodeNumber);

    int firstAdress = beingWorkedInode->dataPtr[0];
    int secondAdress = beingWorkedInode->dataPtr[1];
//    int singleIndirectPointer = beingWorkedInode->singleIndPtr;
//    int doubleIndirectPointer = beingWorkedInode->doubleIndPtr;

    if(firstAdress != INVALID_PTR  && !searchRecordAndRemoveDir(firstAdress, pathname))
        return SUCCESS;
    else if(secondAdress != INVALID_PTR  && !searchRecordAndRemoveDir(secondAdress, pathname))
        return SUCCESS;

    return ERROR;
}


/*-----------------------------------------------------------------------------
Função:	Altera o diretório atual de trabalho (working directory).
		O caminho desse diretório é informado no parâmetro "pathname".
		São considerados erros:
			(a) qualquer situação que impeça a realização da operação
			(b) não existência do "pathname" informado.

Entra:	pathname -> caminho do novo diretório de trabalho.

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
		Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int chdir2 (char *pathname) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // chdir2 // failed initializing\n");
        return ERROR;
    }
    if(!strcmp(pathname, "./") || !strcmp(pathname, ".") )
        return SUCCESS;

    struct t2fs_record* recordOfPath;

    recordOfPath = findRecordOfPath(pathname);

    if(recordOfPath == NULL) {
        fprintf(stderr, "!ERROR! // chdir2 // path '%s' not found\n", pathname);
        return ERROR;
    }

    *currentDirectory = *recordOfPath;

    return SUCCESS;
}


/*-----------------------------------------------------------------------------
Função:	Informa o diretório atual de trabalho.
		O "pathname" do diretório de trabalho deve ser copiado para o buffer indicado por "pathname".
			Essa cópia não pode exceder o tamanho do buffer, informado pelo parâmetro "size".
		São considerados erros:
			(a) quaisquer situações que impeçam a realização da operação
			(b) espaço insuficiente no buffer "pathname", cujo tamanho está informado por "size".

Entra:	pathname -> buffer para onde copiar o pathname do diretório de trabalho
		size -> tamanho do buffer pathname

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
		Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getcwd2 (char *pathname, int size) { //DONE
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // getcwd2 // failed initializing\n");
        return ERROR;
    }

    getPathToDirectory(currentDirectory, pathname, -1);

    return SUCCESS;
}


/*-----------------------------------------------------------------------------
Função:	Abre um diretório existente no disco.
	O caminho desse diretório é aquele informado pelo parâmetro "pathname".
	Se a operação foi realizada com sucesso, a função:
		(a) deve retornar o identificador (handle) do diretório
		(b) deve posicionar o ponteiro de entradas (current entry) na primeira posição válida do diretório "pathname".
	O handle retornado será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do diretório.

Entra:	pathname -> caminho do diretório a ser aberto

Saída:	Se a operação foi realizada com sucesso, a função retorna o identificador do diretório (handle).
	Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
DIR2 opendir2 (char *pathname) { //DONE
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // opendir2 // failed initializing\n");
        return ERROR;
    }

    struct t2fs_record* recordOfPath;
    recordOfPath = findRecordOfPath(pathname);

    if(recordOfPath == NULL) {
        fprintf(stderr, "!ERROR! // opendir2 // path not found\n");
        return ERROR;
    }

    int index;
    int emptySpace = -1;
    for(index = 0; index < MAX_OPEN_DIRECTORIES; index++) {
        if(openedDirectories[index].valid == true) {
            if(openedDirectories[index].directoryRecord == recordOfPath) {
                fprintf(stderr, "!ERROR! // opendir2 // directory already opened\n");
                return ERROR;
            }
        }
        else if(emptySpace == -1) {
            emptySpace = index;
            break;
        }
    }

    if(index >= 16) {
        fprintf(stderr, "!ERROR! // opendir2 // no empty space in opened directories\n");
        return ERROR;
    }

    struct openDirectory* newOpenedDirectory = malloc(sizeof(struct openDirectory));
    newOpenedDirectory->directoryRecord = malloc(sizeof(struct t2fs_record));

    getInodeToBeingWorkedInode(recordOfPath->inodeNumber);

    newOpenedDirectory->valid = true;
    newOpenedDirectory->seekPointer = 0;
    newOpenedDirectory->bytesFileSize = beingWorkedInode->bytesFileSize;
    *newOpenedDirectory->directoryRecord = *recordOfPath;

    openedDirectories[emptySpace] = *newOpenedDirectory;

    free(newOpenedDirectory);

    return emptySpace;
}


/*-----------------------------------------------------------------------------
Função:	Realiza a leitura das entradas do diretório identificado por "handle".
	A cada chamada da função é lida a entrada seguinte do diretório representado pelo identificador "handle".
	Algumas das informações dessas entradas devem ser colocadas no parâmetro "dentry".
	Após realizada a leitura de uma entrada, o ponteiro de entradas (current entry) deve ser ajustado para a próxima entrada válida, seguinte à última lida.
	São considerados erros:
		(a) qualquer situação que impeça a realização da operação
		(b) término das entradas válidas do diretório identificado por "handle".

Entra:	handle -> identificador do diretório cujas entradas deseja-se ler.
	dentry -> estrutura de dados onde a função coloca as informações da entrada lida.

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero ( e "dentry" não será válido)
-----------------------------------------------------------------------------*/
int readdir2 (DIR2 handle, DIRENT2 *dentry) { //DONE
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // readdir2 // failed initializing\n");
        return ERROR;
    }

    if(!isDirectoryHandleValid(handle)) {
        fprintf(stderr, "!ERROR! // readdir2 // invalid handle\n");
        return ERROR;
    }

    struct t2fs_record* records;
    int seekPointer = openedDirectories[handle].seekPointer;

    getInodeToBeingWorkedInode(openedDirectories[handle].directoryRecord->inodeNumber);
    if(seekPointer <= 16)
        records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[0]);
    else if(seekPointer > 16 && seekPointer <= 32)
        records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[1]);
    else if(seekPointer > 32 && seekPointer <= 256)
        return ERROR;
    else if(seekPointer > 256)
        return ERROR;

    if(records[seekPointer].TypeVal != TYPEVAL_REGULAR &&
       records[seekPointer].TypeVal != TYPEVAL_DIRETORIO) {
        return ERROR;
    }

    dentry->fileType = records[seekPointer].TypeVal;
    strncpy(dentry->name, records[seekPointer].name, strlen(records[seekPointer].name)+1);

    getInodeToBeingWorkedInode(records[seekPointer].inodeNumber);
    dentry->fileSize = beingWorkedInode->bytesFileSize;

    openedDirectories[handle].seekPointer++;

    return SUCCESS;
}


/*-----------------------------------------------------------------------------
Função:	Fecha o diretório identificado pelo parâmetro "handle".

Entra:	handle -> identificador do diretório que se deseja fechar (encerrar a operação).

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int closedir2 (DIR2 handle) {
    if(checkInitialization()) {
        fprintf(stderr, "!ERROR! // closedir2 // failed initializing\n");
        return ERROR;
    }

    if(!isDirectoryHandleValid(handle)) {
        fprintf(stderr, "!ERROR! // closedir2 // invalid handle\n");
        return ERROR;
    }

    if(!openedDirectories[handle].valid) {
        fprintf(stderr, "!ERROR! // closedir2 // handle is marked as invalid\n");
        return ERROR;
    }

    if(openedDirectories[handle].directoryRecord == NULL) {
        fprintf(stderr, "!ERROR! // closedir2 // directory record is not a pointer\n");
        return ERROR;
    }

    openedDirectories[handle].valid = false;
    free(openedDirectories[handle].directoryRecord);

    return SUCCESS;
}

bool printRecords(struct t2fs_record* records) {
    if (records == NULL) {
        fprintf(stderr, "!ERROR! // printRecords // records NULL\n");
        return ERROR;
    }

    int index;

    for(index = 0; index < 16; index++) {
        if(records[index].TypeVal == TYPEVAL_REGULAR || records[index].TypeVal == TYPEVAL_DIRETORIO ) //&& beingWorkedRecord->inodeNumber != INVALID_PTR
        {
         printf("Type Value: \t0x0%X (0x01 file, 0x02 dir)\n" ,records[index].TypeVal);
         printf("name: \t\t%s\n", records[index].name);
         printf("i-node-number:  %d\n", records[index].inodeNumber);
         putchar('\n');
        }
    }

    return SUCCESS;
}

bool isFileHandleValid(FILE2 handle) {
    if (handle >= 0 && handle <= MAX_OPEN_FILES)
        return true;
    else
        return false;
}

bool isDirectoryHandleValid(DIR2 handle) {
    if (handle >= 0 && handle <= MAX_OPEN_DIRECTORIES)
        return true;
    else
        return false;
}

bool isOpenedFileStructureValid(FILE2 handle) {
    if(!openedFiles[handle].valid || openedFiles[handle].currentPointer < 0 ||
       openedFiles[handle].fileInode == NULL || openedFiles[handle].fileRecord == NULL) {
        return ERROR;
       }
    return SUCCESS;
}

bool isOpenedDirectoryStructureValid(DIR2 handle) {
    if(!openedDirectories[handle].valid || openedDirectories[handle].bytesFileSize < 0 ||
       openedDirectories[handle].directoryRecord == NULL ){
        return ERROR;
       }
    return SUCCESS;
}

bool checkInitialization() {
    if(!initialized) {
        controller = initializeLibrary();

        if(controller == NULL){
            return ERROR;
        }

        initialized = true;
    }

    return SUCCESS;
}

void printCurrentWorkingDirectory() {
    char pathname[MAX_PATH_LENGTH];
    getcwd2(pathname, sizeof(pathname));
    printf("current working directory: %s\n", pathname);
}

bool findAndAllocateBitmapsForNewFile (int* vacantBlock, int* vacantInode){
    *vacantBlock = searchBitmap2(BITMAP_DADOS, 0);

    unsigned char block[1024];
    memset(block, 0, sizeof(block));
    if(writeBlockToBlockDataSection(block, *vacantBlock)){
        fprintf(stderr, "!ERROR! // findBitmapsForNewFile // failed writing data block\n");
        return ERROR;
    }

    *vacantInode = searchBitmap2(BITMAP_INODE, 0);

    struct t2fs_inode* newInode = malloc(sizeof(struct t2fs_inode));
    newInode->blocksFileSize = 1;
    newInode->bytesFileSize = 1;
    newInode->dataPtr[0] = *vacantBlock;
    newInode->dataPtr[1] = INVALID_PTR;
    newInode->singleIndPtr = INVALID_PTR;
    newInode->doubleIndPtr = INVALID_PTR;
    newInode->reservado[0] = 0;
    newInode->reservado[1] = 0;

    if(writeInodeToInodeDataSection(newInode, *vacantInode)) {
        fprintf(stderr, "!ERROR! // findBitmapsForNewFile // failed writing inode line\n");
        return ERROR;
    }

    setBitmap2(BITMAP_DADOS, *vacantBlock, 1);
    setBitmap2(BITMAP_INODE, *vacantInode, 1);

    free(newInode);

    return SUCCESS;
}

bool findAndAllocateBitmapsForNewDirectory (int* vacantBlock, int* vacantInode){
    *vacantBlock = searchBitmap2(BITMAP_DADOS, 0);

    unsigned char block[1024];
    memset(block, 0, sizeof(block));
    if(writeBlockToBlockDataSection(block, *vacantBlock)){
        fprintf(stderr, "!ERROR! // findAndAllocateBitmapsForNewDirectory // failed writing data block\n");
        return ERROR;
    }

    *vacantInode = searchBitmap2(BITMAP_INODE, 0);

    struct t2fs_inode* newInode = malloc(sizeof(struct t2fs_inode));
    newInode->blocksFileSize = 1;
    newInode->bytesFileSize = 1024;
    newInode->dataPtr[0] = *vacantBlock;
    newInode->dataPtr[1] = INVALID_PTR;
    newInode->singleIndPtr = INVALID_PTR;
    newInode->doubleIndPtr = INVALID_PTR;
    newInode->reservado[0] = 0;
    newInode->reservado[1] = 0;

    if(writeInodeToInodeDataSection(newInode, *vacantInode)) {
        fprintf(stderr, "!ERROR! // findAndAllocateBitmapsForNewDirectory // failed writing inode line\n");
        return ERROR;
    }

    setBitmap2(BITMAP_DADOS, *vacantBlock, 1);
    setBitmap2(BITMAP_INODE, *vacantInode, 1);

    free(newInode);

    return SUCCESS;
}

bool searchRecordAndRemoveDir(DWORD dataPointer, char* pathname) {
    if(dataPointer == 0 || pathname == NULL) {
        fprintf(stderr, "!ERROR! // searchRecordAndRemoveDir // invalid arguments\n");
        return ERROR;
    }

    struct t2fs_record* records;
    records = inodeDataPointerToRecords(dataPointer);

    int index;
    for(index = 2; index < 16; index++) {
        if(records[index].TypeVal != TYPEVAL_INVALIDO) {
            fprintf(stderr, "!ERROR! // searchRecordAndRemoveDir // directory not empty\n");
            return ERROR;
        }
    }

    struct t2fs_record* recordOfFatherDir = malloc(sizeof(struct t2fs_record));
    *recordOfFatherDir = records[1];

    int adressPointer = -1;

    char nameOfBeingDeletedDir[MAX_PATH_LENGTH];
    getNameOfDirectoryAtEndOfPath(pathname, nameOfBeingDeletedDir);

    if(recordOfFatherDir == NULL || recordOfFatherDir->TypeVal != TYPEVAL_DIRETORIO){
            fprintf(stderr, "!ERROR! // searchRecordAndRemoveDir // record of father dir wasn't found\n");
            free(recordOfFatherDir);
            return ERROR;
    }

    getInodeToBeingWorkedInode(recordOfFatherDir->inodeNumber);
    DWORD firstAdress = beingWorkedInode->dataPtr[0];
    DWORD secondAdress = beingWorkedInode->dataPtr[1];

    records = inodeDataPointerToRecords(firstAdress);

    for(index = 2; index < 16; index++) {
        if(!strcmp(records[index].name,nameOfBeingDeletedDir)){
            adressPointer = firstAdress;
            break;
        }
    }

    if(adressPointer == -1) {
        records = inodeDataPointerToRecords(secondAdress);
        for(index = 2; index < 16; index++) {
            if(!strcmp(records[index].name,nameOfBeingDeletedDir)){
                adressPointer = secondAdress;
                break;
            }
        }
    }

    if(adressPointer < 0){
        fprintf(stderr, "!ERROR! // searchRecordAndRemoveDir // directory not found in direct pointers of inode\n");
        free(recordOfFatherDir);
        return ERROR;
    }


    if(&records[index] == NULL) {
        fprintf(stderr, "!ERROR! // searchRecordAndRemoveDir // directory not found in parent directory\n");
        free(recordOfFatherDir);
        return ERROR;
    }

    unsigned char bufferBlock[1024];
    inodeFillRecordsToBufferBlock(adressPointer, bufferBlock);
    inodeRemoveRecordAndReturnBufferBlock(adressPointer, nameOfBeingDeletedDir, bufferBlock);

    if(writeBlockToBlockDataSection(bufferBlock, adressPointer)) {
        fprintf(stderr, "!ERROR! // searchRecordAndRemoveDir // failed to write updated block\n");
        free(recordOfFatherDir);
        return ERROR;
    }

    getInodeToBeingWorkedInode(records[index].inodeNumber);
    DWORD dataPointerToRecordsOfDir = beingWorkedInode->dataPtr[0];
    DWORD secondDataPointerToRecordsOfDir = beingWorkedInode->dataPtr[1];
    DWORD singleIndrDataPointerToRecordsOfDir = beingWorkedInode->singleIndPtr;
    DWORD doubleIndrDataPointerToRecordsOfDir = beingWorkedInode->doubleIndPtr;

    if(dataPointerToRecordsOfDir != INVALID_PTR)
        setBitmap2(BITMAP_DADOS, dataPointerToRecordsOfDir, 0);

    if(secondDataPointerToRecordsOfDir != INVALID_PTR)
        setBitmap2(BITMAP_DADOS, secondDataPointerToRecordsOfDir, 0);

    if(singleIndrDataPointerToRecordsOfDir != INVALID_PTR)
        setBitmap2(BITMAP_DADOS, singleIndrDataPointerToRecordsOfDir, 0);

    if(doubleIndrDataPointerToRecordsOfDir != INVALID_PTR)
        setBitmap2(BITMAP_DADOS, doubleIndrDataPointerToRecordsOfDir, 0);

    setBitmap2(BITMAP_INODE, records[index].inodeNumber, 0);

    free(recordOfFatherDir);
    return SUCCESS;
}

bool searchRecordAndRemoveFile(DWORD dataPointer, char* filename) {
    if(dataPointer == 0 || filename == NULL) {
        fprintf(stderr, "!ERROR! // searchRecordAndRemoveFile // invalid arguments\n");
        return ERROR;
    }

    struct t2fs_record* records;
    records = inodeDataPointerToRecords(dataPointer);

    int index;
    for(index = 2; index < 16; index++) {
        if(!strcmp(records[index].name, filename) && records[index].TypeVal == TYPEVAL_REGULAR)
            break;
    }

    if(index >= 16) {
        fprintf(stderr, "!ERROR! // searchRecordAndRemoveFile // file not found in directory\n");
        return ERROR;
    }

    unsigned char bufferBlock[1024];
    inodeFillRecordsToBufferBlock(dataPointer, bufferBlock);
    inodeRemoveRecordAndReturnBufferBlock(dataPointer, filename, bufferBlock);

    if(writeBlockToBlockDataSection(bufferBlock, dataPointer)) {
        fprintf(stderr, "!ERROR! // searchRecordAndRemoveFile // failed to write updated block\n");
        return ERROR;
    }

    getInodeToBeingWorkedInode(records[index].inodeNumber);
    setBitmap2(BITMAP_DADOS, beingWorkedInode->dataPtr[0], 0);
    setBitmap2(BITMAP_INODE, records[index].inodeNumber, 0);

    return SUCCESS;
}


int searchVacantSpaceAndCreateFile(DWORD dataPointer, char* filename, char* fullPath) {
    struct t2fs_record* records;
    records = inodeDataPointerToRecords(dataPointer);

    int index;
    for(index = 2; index < 16; index++) {
        if(records[index].TypeVal != TYPEVAL_INVALIDO && !strcmp(records[index].name, filename)) {
            fprintf(stderr, "!ERROR! // searchVacantSpaceAndCreateFile // file already is on directory\n");
            return ERROR;
        }
    }
    int vacantBlock;
    int vacantInode;

    findAndAllocateBitmapsForNewFile(&vacantBlock, &vacantInode);

    struct t2fs_record* newRecord = malloc(sizeof(struct t2fs_record));
    newRecord->TypeVal = 1;
    strcpy(newRecord->name,filename);
    newRecord->inodeNumber = vacantInode;

    unsigned char bufferBlock[1024];
    inodeFillRecordsToBufferBlock(dataPointer, bufferBlock);

    bool errorCode = inodeAppendRecordToBufferBlock(dataPointer, newRecord, bufferBlock);
    if(errorCode) {
        free(newRecord);
        fprintf(stderr, "!ERROR! // searchVacantSpaceAndCreateFile // error appending record\n");
        return ERROR;
    }

    writeBlockToBlockDataSection(bufferBlock, dataPointer);

    free(newRecord);

    int fileHandle = open2(fullPath);
    if(fileHandle == -1) {
        fprintf(stderr, "!ERROR! // searchVacantSpaceAndCreateFile // tried to open created file and failed\n");
        return ERROR;
    }

    return fileHandle;
}

bool makeRecordsForNewDir(struct t2fs_record* recordOfParentDirectory, char* pathname) {
    char newDirectoryName[59];

    char* position = rstrstr(pathname, "/");
    if(position == NULL && recordOfParentDirectory->inodeNumber != 0) {
        fprintf(stderr, "!ERROR! // makeRecordsForNewDir // pathname is invalid and isn't root\n");
        return ERROR;
    }

    if(position == pathname){
        strncpy(newDirectoryName, pathname+1, strlen(pathname));
    }
    else if(position == NULL) {
        strncpy(newDirectoryName, pathname, strlen(pathname)+1);
    }
    else{
        subString(pathname, newDirectoryName, (int) (position - pathname) + 1, (int)strlen(pathname) - (int) (position - pathname) -1);
    }

    struct t2fs_record* records;
    getInodeToBeingWorkedInode(recordOfParentDirectory->inodeNumber);

    int firstAdress = beingWorkedInode->dataPtr[0];
    int secondAdress = beingWorkedInode->dataPtr[1];
//    int singleIndirectPointer = beingWorkedInode->singleIndPtr;
//    int doubleIndirectPointer = beingWorkedInode->doubleIndPtr;

    bool vacancyInFirstAdress = false;

    int index;
    if(firstAdress != INVALID_PTR) {
        records = inodeDataPointerToRecords(firstAdress);

        for(index = 2; index < 16; index++) {
            if(records[index].TypeVal != TYPEVAL_INVALIDO &&!strcmp(records[index].name, newDirectoryName)) {
                fprintf(stderr, "!ERROR! // makeRecordsForNewDir // directory with same name is target directory\n");
                return ERROR;
            }
            if(records[index].TypeVal == TYPEVAL_INVALIDO && !vacancyInFirstAdress)
                vacancyInFirstAdress = true;
        }
    }

    if(secondAdress != INVALID_PTR) {
        records = inodeDataPointerToRecords(firstAdress);

        for(index = 2; index < 16; index++) {
            if(!strcmp(records[index].name, newDirectoryName)) {
                fprintf(stderr, "!ERROR! // makeRecordsForNewDir // directory with same name is target directory\n");
                return ERROR;
            }
        }

    }

    int vacantBlock;
    int vacantInode;

    findAndAllocateBitmapsForNewDirectory(&vacantBlock, &vacantInode);
    unsigned char bufferBlock[1024];
    prepareNewRecordsBlockBuffer(bufferBlock);
    writeBlockToBlockDataSection(bufferBlock, vacantBlock);

    struct t2fs_record* newRecord = malloc(sizeof(struct t2fs_record));
    newRecord->TypeVal = TYPEVAL_DIRETORIO;
    strncpy(newRecord->name,".\0", 2);
    newRecord->inodeNumber = vacantInode;

    bool errorCode = inodeAppendRecordToBufferBlock(vacantBlock, newRecord, bufferBlock);
    if(errorCode) {
        free(newRecord);
        fprintf(stderr, "!ERROR! // makeRecordsForNewDir // error appending record\n");
        return ERROR;
    }

    writeBlockToBlockDataSection(bufferBlock, vacantBlock);

    newRecord->TypeVal = TYPEVAL_DIRETORIO;
    strncpy(newRecord->name,"..\0",3);
    newRecord->inodeNumber = records[0].inodeNumber;

    errorCode = inodeAppendRecordToBufferBlock(vacantBlock, newRecord, bufferBlock);

    if(errorCode) {
        free(newRecord);
        fprintf(stderr, "!ERROR! // makeRecordsForNewDir // error appending record\n");
        return ERROR;
    }

    writeBlockToBlockDataSection(bufferBlock, vacantBlock);

    newRecord->TypeVal = TYPEVAL_DIRETORIO;
    strncpy(newRecord->name,newDirectoryName,strlen(newDirectoryName)+1);
    newRecord->inodeNumber = vacantInode;

    int dataPointer = -1;
    memset(bufferBlock, 0, sizeof(bufferBlock));
    if(vacancyInFirstAdress) {
        inodeFillRecordsToBufferBlock(firstAdress, bufferBlock);
        errorCode = inodeAppendRecordToBufferBlock(firstAdress, newRecord, bufferBlock);
        dataPointer = firstAdress;
    }
    else {
        inodeFillRecordsToBufferBlock(secondAdress, bufferBlock);
        errorCode = inodeAppendRecordToBufferBlock(secondAdress, newRecord, bufferBlock);
        dataPointer = secondAdress;
    }

    if(errorCode) {
        free(newRecord);
        fprintf(stderr, "!ERROR! // makeRecordsForNewDir // error appending record\n");
        return ERROR;
    }

    if(writeBlockToBlockDataSection(bufferBlock, dataPointer)) {
        fprintf(stderr, "!ERROR! // makeRecordsForNewDir // failed to write updated block\n");
        return ERROR;
    }

    setBitmap2(BITMAP_DADOS, vacantBlock, 1);
    setBitmap2(BITMAP_INODE, vacantInode, 1);

    free(newRecord);

    return SUCCESS;
}
