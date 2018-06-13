#include <stdio.h>
#include <stdlib.h>
#include "../include/t2fs.h"
#include "../include/directory.h"
#include "../include/initializer.h"

int main()
{
    printf("Hello world!\n");

    char groupName[SIZEIDENTIFY+1];
    int errorCode = identify2(groupName, SIZEIDENTIFY);
    if(errorCode == -1) {
        printf("Main error");
        return -1;
    }

    printDirectoryTree();

    printf("\t##TESTE## mudando de diretorio para dir1\n");
    chdir2("dir1/");
    printf("\t##TESTE## mudando de diretorio para diretorio nao existente\n");
    printCurrentWorkingDirectory();
    chdir2("GLABHERGHARBLE/");

    printf("\t##TESTE## mudando de diretorio para diretorio nao existente\n");
    chdir2("../");
    printCurrentWorkingDirectory();

    printf("\t##TESTE## abrindo diretorio1\n");
    opendir2("./dir1");
    printf("\t##TESTE## fechando diretorio1\n");
    closedir2(0);
    printf("\t##TESTE## refazendo operação\n");
    opendir2("./dir1");
    closedir2(0);

    printf("\t##TESTE## abrindo arquivo file3\n");
    open2("file3");
    char buffer[1024];
    printf("\t##TESTE## lendo do arquivo file3\n");
    read2(0,buffer,1024);

    printf("buffer: %s\n", buffer);

    printf("\t##TESTE## criando novo arquivo\n");
    create2("pipi");
    delete2("pipi");

    printf("\t##TESTE## criando novo diretorios\n");
    mkdir2("/dir666");
    mkdir2("/dir666/dir667");

    printf("\t##TESTE## imprimindo arvore de diretorios\n");
    printDirectoryTree();
    printf("\t##TESTE## removendo diretorio\n");
    //rmdir2("/dir668");
    printf("\t##TESTE## imprimindo arvore de diretorios novamente\n");
    printDirectoryTree();

    printf("\t##TESTE## imprimindo arvore de diretorios novamente\n");
    printDirectoryTree();

    printf("\t##TESTE## mudando para ./dir2/dir21/\n");
    chdir2("./dir2/dir21/");
    printCurrentWorkingDirectory();

    printf("\t##TESTE## mudando para ..\n");
    chdir2("../");
    printCurrentWorkingDirectory();

    printf("Nome do grupo:\n%s\n", groupName);

    unintializeLibrary();

    return 0;
}
