#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src

all: blockhandler directory initializer inodehandler stringfunctions t2fs lct2fs

lct2fs: $(BIN_DIR)/t2fs.o $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o
	ar rcs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/t2fs.o $(BIN_DIR)/blockhandler.o $(BIN_DIR)/directory.o $(BIN_DIR)/initializer.o $(BIN_DIR)/inodehandler.o $(BIN_DIR)/stringfunctions.o

blockhandler: $(SRC_DIR)/blockhandler.c $(INC_DIR)/t2fs.h $(INC_DIR)/apidisk.h  $(INC_DIR)/bitmap2.h    $(INC_DIR)/blockhandler.h  $(INC_DIR)/initializer.h $(INC_DIR)/inodehandler.h
	$(CC) -c $(SRC_DIR)/blockhandler.c -o $(BIN_DIR)/blockhandler.o

directory: $(SRC_DIR)/directory.c $(INC_DIR)/directory.h $(INC_DIR)/blockhandler.h $(INC_DIR)/stringfunctions.h  $(INC_DIR)/initializer.h $(INC_DIR)/inodehandler.h
	$(CC) -c $(SRC_DIR)/directory.c -o $(BIN_DIR)/directory.o

initializer: $(SRC_DIR)/initializer.c $(INC_DIR)/t2fs.h $(INC_DIR)/apidisk.h  $(INC_DIR)/bitmap2.h  $(INC_DIR)/initializer.h  $(INC_DIR)/directory.h $(INC_DIR)/inodehandler.h
	$(CC) -c $(SRC_DIR)/initializer.c -o $(BIN_DIR)/initializer.o

inodehandler: $(SRC_DIR)/inodehandler.c $(INC_DIR)/t2fs.h $(INC_DIR)/inodehandler.h $(INC_DIR)/blockhandler.h
	$(CC) -c $(SRC_DIR)/inodehandler.c -o $(BIN_DIR)/inodehandler.o

stringfunctions: $(SRC_DIR)/stringfunctions.c
	$(CC) -c $(SRC_DIR)/stringfunctions.c -o $(BIN_DIR)/stringfunctions.o

t2fs: $(SRC_DIR)/t2fs.c $(INC_DIR)/t2fs.h $(INC_DIR)/apidisk.h $(INC_DIR)/directory.h  $(INC_DIR)/blockhandler.h  $(INC_DIR)/inodehandler.h $(INC_DIR)/initializer.h $(INC_DIR)/stringfunctions.h 
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~


