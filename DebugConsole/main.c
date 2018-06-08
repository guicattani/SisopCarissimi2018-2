#include <stdio.h>
#include <stdlib.h>
#include "../include/t2fs.h"

int main()
{
    printf("Hello world!\n");

    char groupName[SIZEIDENTIFY+1];
    int errorCode = identify2(groupName, SIZEIDENTIFY);
    if(errorCode == -1) {
        printf("Main error");
        return -1;
    }

    printCurrentWorkingDirectory();
    chdir2("./");
    printCurrentWorkingDirectory();
    chdir2("../");
    printCurrentWorkingDirectory();

    chdir2("dir11/");
    printCurrentWorkingDirectory();

    chdir2("/");
    printCurrentWorkingDirectory();

    opendir2("./dir1");
    closedir2(0);
    opendir2("./dir1");
    closedir2(0);



    printf("Nome do grupo:\n%s\n", groupName);

    return 0;
}
