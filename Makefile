CUR_PATH = $(shell pwd)
COMMON_PATH = ${CUR_PATH}/../common

SRC = ${wildcard  ${CUR_PATH}/*.c}

OBJS = ${patsubst %.c, %.o, ${SRC}}
TARGET=test_g
CC=gcc
CCFLAGS=-g -Wall
INC_DIR=-I.
LIB_DIR=-L.

LIBS=-lavformat -lavutil

RM=rm -f

${TARGET}: ${OBJS}
	${CC} ${CCFLAGS} ${OBJS} -o $@  ${LIB_DIR} $(LIBS)

$(OBJS):%.o:%.c
	${CC} ${CCFLAGS} ${INC_DIR} -c $< -o $@ $(LIBS)

.PHONY:clean
clean:
	$(RM) $(OBJS) ${TARGET}