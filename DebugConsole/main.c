#include <stdio.h>
#include <stdlib.h>
#include "../include/t2fs.h"

#define SIZEIDENTIFY 67

int main()
{
    printf("Hello world!\n");

    char groupName[SIZEIDENTIFY+1];
    identify2 (groupName, SIZEIDENTIFY);

    printf ("Nome do grupo:\n%s\n", groupName);

    return 0;
}
