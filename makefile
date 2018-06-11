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

lct2fs: $(BIN_DIR)/t2fs.o $(BIN_DIR)/apidisk.o $(BIN_DIR)/bitmap2.o
	ar rcs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/t2fs.o $(BIN_DIR)/blockhandler.o $(BIN_DIR)/directory.o $(BIN_DIR)/initializer.o $(BIN_DIR)/inodehandler.o $(BIN_DIR)/stringfunctions.o

blockhandler: $(SRC_DIR)/blockhandler.c $(INC_DIR)/blockhandler.h $(INC_DIR)/cdata.h
	$(CC) -c $(SRC_DIR)/blockhandler.c -o $(BIN_DIR)/blockhandler.o

directory: $(SRC_DIR)/scheduler.c $(INC_DIR)/support.h $(INC_DIR)/cdata.h
	$(CC) -c $(SRC_DIR)/scheduler.c -o $(BIN_DIR)/scheduler.o

initializer: $(SRC_DIR)/cthread.c $(INC_DIR)/support.h $(INC_DIR)/scheduler.h $(INC_DIR)/cthread.h $(INC_DIR)/cdata.h
	$(CC) -c $(SRC_DIR)/cthread.c $(BIN_DIR)/scheduler.o -o $(BIN_DIR)/cthread.o

inodehandler: $(SRC_DIR)/cthread.c $(INC_DIR)/support.h $(INC_DIR)/scheduler.h $(INC_DIR)/cthread.h $(INC_DIR)/cdata.h
	$(CC) -c $(SRC_DIR)/cthread.c $(BIN_DIR)/scheduler.o -o $(BIN_DIR)/cthread.o

stringfunctions: $(SRC_DIR)/cthread.c $(INC_DIR)/support.h $(INC_DIR)/scheduler.h $(INC_DIR)/cthread.h $(INC_DIR)/cdata.h
	$(CC) -c $(SRC_DIR)/cthread.c $(BIN_DIR)/scheduler.o -o $(BIN_DIR)/cthread.o

t2fs: $(SRC_DIR)/cthread.c $(INC_DIR)/support.h $(INC_DIR)/scheduler.h $(INC_DIR)/cthread.h $(INC_DIR)/cdata.h
	$(CC) -c $(SRC_DIR)/cthread.c $(BIN_DIR)/scheduler.o -o $(BIN_DIR)/cthread.o


clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~


