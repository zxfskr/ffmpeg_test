CUR_PATH = $(shell pwd)
COMMON_PATH = ${CUR_PATH}/../../common

SRC = ${wildcard  ${CUR_PATH}/*.c}

OBJS = ${patsubst %.c, %.o, ${SRC}}
TARGET=test_g
CC=gcc
CCFLAGS=-g -Wall
INC_DIR=-I${COMMON_PATH}/include
LIB_DIR=-L${COMMON_PATH}/lib

LIBS=-lrtmp -lssl -lcrypto -lz

RM=rm -f

${TARGET}: ${OBJS}
	${CC} ${OBJS} -o $@  ${LIB_DIR} $(LIBS)

$(OBJS):%.o:%.c
	${CC} ${CCFLAGS} ${INC_DIR} -c $< -o $@

.PHONY:clean
clean:
	$(RM) $(OBJS) ${TARGET}