#include <stdio.h>
#include <stdlib.h>
#include "../include/t2fs.h"

#define SIZEIDENTIFY 67

int main()
{
    printf("Hello world!\n");

    char groupName[SIZEIDENTIFY+1];
    int errorCode = identify2 (groupName, SIZEIDENTIFY);

    if(errorCode == ERROR) {
        printf("Main error");
        return -1;
    }

    printf ("Nome do grupo:\n%s\n", groupName);

    return 0;
}
