#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"

#include "../include/initializer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool initializeLibrary(){
    printf("OLAR PESSOA PARTE DOIS");

    bootFileSystem();
    return SUCCESS;
}

bool bootFileSystem(){

    unsigned char buffer[256];
	int index = 0;

//	if(read_sector(0, buffer) != 0){
//		fprintf(stderr, "Falha ao ler setor do disco.\n");
//		return ERROR;
//	}
//
//    else
//        printf("LEU DISCO");

    return SUCCESS;
}
