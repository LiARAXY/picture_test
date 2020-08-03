TARGET=picture_test
EXTRA_LIB_DIR=/usr/local/armv7-lib-extra
CUR_DIR=$(shell pwd)
DIR_SRC=$(CUR_DIR)/src
DIR_OBJ=$(CUR_DIR)/obj
DIR_OUT=$(CUR_DIR)/bin
INC=-I$(EXTRA_LIB_DIR)/include -I$(CUR_DIR)/include
LIB=-L$(EXTRA_LIB_DIR)/lib
LFLAGS=-lm -lfreetype -lcharset -liconv -ljpeg -lpng -lz

SRC = ${wildcard ${DIR_SRC}/*c}
OBJ = $(patsubst %.c, ${DIR_OBJ}/%.o, $(notdir ${SRC}))
BIN_TARGET = ${DIR_OUT}/$(TARGET)
CC=$(CROSS_COMPILE)gcc
CFLAGS=-g -Wall
$(BIN_TARGET):${OBJ}
	@echo "Compile target."
	$(CC) $(CFLAGS) ${INC} ${LIB} $^ -o $@ $(LFLAGS) 
${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	@echo "Compile src obj."
	$(shell if [ ! -d $(DIR_OBJ) ]; then mkdir $(DIR_OBJ); fi)
	$(shell if [ ! -d $(DIR_OUT) ]; then mkdir $(DIR_OUT); fi)
	$(CC) $(CFLAGS) ${INC} ${LIB} -c $< -o $@ $(LFLAGS)
.PHONY:clean
clean:
	find ${DIR_OBJ} -name *.o -exec rm -rf {}  \;
	rm -rf ${DIR_OUT}/${TARGET}
.PHONY:distclean
distclean:
	rm -rf ${DIR_OBJ}
	rm -rf ${DIR_OUT}