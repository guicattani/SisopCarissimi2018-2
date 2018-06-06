#include "../include/t2fs.h"
#include "../include/initializer.h"
#include "../include/directory.h"
#include "../include/inodehandler.h"
#include "../include/blockhandler.h"

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
   if(checkInitialization())
        return ERROR;

    if(size < SIZEIDENTIFY)
        return ERROR;

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
    if(checkInitialization())
        return ERROR;

    if(filename == NULL)
        return ERROR;

    getInodeToBeingWorkedInode(currentDirectory->inodeNumber);

    if(beingWorkedInode->blocksFileSize > 1) //usa dataptr[1]
        return ERROR; //TODO
    if(beingWorkedInode->blocksFileSize > 2)
        return ERROR; //TODO

    struct t2fs_record* records;
    records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[0]);

    int index;
    for(index = 2; index < 16; index++) {
        if(strcmp(records[index].name, filename))
            return ERROR;
    }
    int vacantBlock = searchBitmap2(controller->freeBlockBitmap, 0);

    unsigned char block[1024];
    memset(block, 0, sizeof(block));
    //writeBlockToBlockDataSector(block, vacantBlock);

    int vacantInode = searchBitmap2(controller->freeInodeBitmap, 0);

    struct t2fs_inode* newInode = malloc(sizeof(struct t2fs_inode));
    newInode->blocksFileSize = 1;
    newInode->bytesFileSize = 1;
    newInode->dataPtr[0] = vacantBlock + dataSectionInBlocks;
    newInode->dataPtr[1] = 0;
    newInode->singleIndPtr = 0;
    newInode->doubleIndPtr = 0;
    newInode->reservado[0] = 0;
    newInode->reservado[1] = 0;

    memset(block, 0, sizeof(block));
    memcpy(block, newInode, sizeof(newInode));
    //writeBlockToBlockDataSector(block, vacantInode);

    setBitmap2(controller->freeBlockBitmap, vacantBlock, 1);
    setBitmap2(controller->freeInodeBitmap, vacantInode, 1);

    struct t2fs_record* newRecord = malloc(sizeof(struct t2fs_record));
    newRecord->TypeVal = 1;
    strcpy(newRecord->name,filename);
    newRecord->inodeNumber = vacantInode;

    bool errorCode = inodeAppendRecord(beingWorkedInode->dataPtr[0], newRecord);
    if(errorCode) {
        free(newInode);
        free(newRecord);
        return -1;
    }

    int fileHandle = open2(filename);
    if(fileHandle == -1)
        return ERROR;

    free(newInode);
    free(newRecord);
    return fileHandle;
}


/*-----------------------------------------------------------------------------
Função:	Apagar um arquivo do disco.
	O nome do arquivo a ser apagado é aquele informado pelo parâmetro "filename".

Entra:	filename -> nome do arquivo a ser apagado.

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int delete2 (char *filename) {
    if(checkInitialization())
        return ERROR;

    if(filename == NULL)
        return ERROR;

    getInodeToBeingWorkedInode(currentDirectory->inodeNumber);

    if(beingWorkedInode->blocksFileSize > 1) //usa dataptr[1]
        return ERROR; //TODO
    if(beingWorkedInode->blocksFileSize > 2)
        return ERROR; //TODO

    struct t2fs_record* records;
    records = inodeDataPointerToRecords(beingWorkedInode->dataPtr[0]);

    int index;
    for(index = 2; index < 16; index++) {
        if(strcmp(records[index].name, filename))
            break;
    }

    if(index == 16)
        return ERROR;

    getInodeToBeingWorkedInode(records[index].inodeNumber);

    if(beingWorkedInode->blocksFileSize > 1) //usa dataptr[1]
        return ERROR; //TODO
    if(beingWorkedInode->blocksFileSize > 2)
        return ERROR; //TODO

    setBitmap2(controller->freeBlockBitmap, beingWorkedInode->dataPtr[0], 0);
    setBitmap2(controller->freeInodeBitmap, records[index].inodeNumber, 0);

    memset(&records[index], 0, sizeof(struct t2fs_record));

    return SUCCESS;
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
    if(checkInitialization())
        return ERROR;

    //TODO implementação

    /** PRIMEIRO LER ACIMA **/

    //TESTES
    //verifica se arquivo existe no diretorio, vejam o diretory.c pra funções pra ajudar nisso
    //checar se a estrutura de arquivos abertos openedFiles não está cheia, VER HEADER T2FS

    //criar uma entrada para a estrutura que guarda quais arquivos estão abertos (openedFiles VER O HEADER T2FS)
    //retorna o número dessa entrada (isso é um handle)

    //cria uma nova entrada para a pra estrutura openedFiles VER HEADER

    return SUCCESS;
}


/*-----------------------------------------------------------------------------
Função:	Fecha o arquivo identificado pelo parâmetro "handle".

Entra:	handle -> identificador do arquivo a ser fechado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle) { //TODO CASOS DE TESTE
    if(checkInitialization())
        return ERROR;

    if(!isFileHandleValid(handle))
        return ERROR;

    /** TODO TESTAR! **/

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
    if(checkInitialization())
        return ERROR;

    if(!isFileHandleValid(handle))
        return ERROR;
    //TODO implementação

    /** PRIMEIRO LER ACIMA **/

    //TESTES
    //testar se o size não é maior que o arquivo

    //achar o arquivo pelo inode, isso tem na estrutura do openedFiles
    //verificar qual o tamanho do size, pra ver se está tudo nos ponteiros diretos (cada ponteiro direto aponta pra um bloco cheio de DWORDs)

    //se não tem que ver como alocar pros ponteiros indiretos

    //le pro buffer com read to working block e aí usa memcpy pro buffer

    //atenção, lembrar que
    //conforme escrito acima:
    //*Após a leitura, o contador de posição (current pointer) deve ser ajustado para o byte seguinte ao último lido.*

    //retorna SUCCESS (0) ou ERROR (-1)

    return SUCCESS;
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
    if(checkInitialization())
        return ERROR;

    if(!isFileHandleValid(handle))
        return ERROR;

    //TODO implementação

    /** PRIMEIRO LER ACIMA **/

    //muito parecido com o read, ver acima

    return SUCCESS;
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
    if(checkInitialization())
        return ERROR;

    if(!isFileHandleValid(handle))
        return ERROR;

    //TODO implementação

    /** PRIMEIRO LER ACIMA **/

    /**cuidado que é INCLUSIVE, então currentpointer é sobrescrito com 0**/
    //corta os bytes sobresalentes

    return SUCCESS;
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
int seek2 (FILE2 handle, DWORD offset) { //TODO CASOS DE TESTE
    if(checkInitialization())
        return ERROR;

    if(!isFileHandleValid(handle) || offset < -1);
        return ERROR;

    /** TODO TESTAR! **/

    if(offset >= 0)
        openedFiles[handle].currentPointer = offset;
    else if (offset == -1)
        openedFiles[handle].currentPointer = openedFiles[handle].fileInode->bytesFileSize - 1;

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
    if(checkInitialization())
        return ERROR;
    //TODO implementação

    /** PRIMEIRO LER ACIMA **/

    //TESTES
    //verificar se o caminho é valido ou se já existe diretorio com esse nome
    //ver directory.c, lá tem as funções pra ajudar no teste

    //muito parecido com create, ver acima
    //mas nesse caso criamos um inode que tem records (entrada de diretorio), cada pointer do inode aponta pra um record

    return SUCCESS;
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
    if(checkInitialization())
        return ERROR;

    //TODO implementação

    /** PRIMEIRO LER ACIMA **/

    //muito parecido com o delete file,  ver acima
    //mas precisa verificar se está vazio, conforme acima

    //também é preciso andar para o diretorio superior(se houver) e deletar a entrada

    return SUCCESS;
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
int chdir2 (char *pathname) { //TODO CASOS DE TESTE
    if(checkInitialization())
        return ERROR;

    /** TODO TESTAR! **/

    struct t2fs_record* recordOfPath;

    recordOfPath = findRecordOfPath(pathname);

    if(recordOfPath && (pathname[0] == '.' && (pathname[1] == '.' || pathname[1] == '/' )) )
        recordOfPath = relativePathExists(pathname, recordOfPath);

    if(recordOfPath == NULL)
        return ERROR;

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
int getcwd2 (char *pathname, int size) { //TODO CASOS DE TESTE
    if(checkInitialization())
        return ERROR;

    /** TODO TESTAR! **/

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
DIR2 opendir2 (char *pathname) { //TODO CASOS DE TESTE
    if(checkInitialization())
        return ERROR;

    /** TODO TESTAR! **/

    struct t2fs_record* recordOfPath;
    recordOfPath = findRecordOfPath(pathname);

    if(recordOfPath == NULL)
        return ERROR;

    int index;
    int emptySpace = -1;
    for(index = 0; index < MAX_OPEN_DIRECTORIES; index++) {
        if(openedDirectories[index].valid == true) {
            if(openedDirectories[index].directoryRecord == recordOfPath) {
                return ERROR;
            }
        }
        else if(emptySpace == -1)
            emptySpace = index;
    }

    struct openDirectory* newOpenedDirectory = malloc(sizeof(struct openDirectory));

    getInodeToBeingWorkedInode(recordOfPath->inodeNumber);

    newOpenedDirectory->valid = true;
    newOpenedDirectory->bytesFileSize = beingWorkedInode->bytesFileSize;
    *newOpenedDirectory->directoryRecord = *recordOfPath;

    openedDirectories[emptySpace] = *newOpenedDirectory;

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
int readdir2 (DIR2 handle, DIRENT2 *dentry) {
    if(checkInitialization())
        return ERROR;

    if(!isDirectoryHandleValid(handle))
        return ERROR;

    /** PRIMEIRO LER ACIMA **/

    //TODO implementação

    //muito parecido com o readfile, ver acima
    //mas tem que usar a estrutura dirent2 que tem no header t2fs.h

    return SUCCESS;
}


/*-----------------------------------------------------------------------------
Função:	Fecha o diretório identificado pelo parâmetro "handle".

Entra:	handle -> identificador do diretório que se deseja fechar (encerrar a operação).

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int closedir2 (DIR2 handle) {//TODO CASOS DE TESTE
    if(checkInitialization())
        return ERROR;

    if(!isDirectoryHandleValid(handle))
        return ERROR;

    /** TODO TESTAR! **/

    openedDirectories[handle].valid = false;

    return SUCCESS;
}

/*-----------------------------------------------------------------------------*/
//TODO documentação
bool printRecords(struct t2fs_record* records) {
    if (records == NULL)
        return ERROR;

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
