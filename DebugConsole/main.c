#include <stdio.h>
#include <stdlib.h>
#include "../include/t2fs.h"
#include "../include/directory.h"

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

    printCurrentWorkingDirectory();
    chdir2("./");
    printCurrentWorkingDirectory();
    chdir2("../");
    printCurrentWorkingDirectory();

    chdir2("dir1/");
    printCurrentWorkingDirectory();
    chdir2("GLABHERGHARBLE/");

    chdir2("../");
    printCurrentWorkingDirectory();

    opendir2("./dir1");
    closedir2(0);
    opendir2("./dir1");
    closedir2(0);

    open2("file3");
    char buffer[1024];
    read2(0,buffer,1024);

    printf("buffer: %s\n", buffer);

    create2("pipi");
    delete2("pipi");

    mkdir2("/dir666");
    delete2("file3");

    chdir2("./dir2/dir21/");
    printCurrentWorkingDirectory();

    delete2("file211");

    chdir2("../");

    rmdir2("dir21");

    printf("Nome do grupo:\n%s\n", groupName);

    return 0;
}
