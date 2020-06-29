CUR_PATH = $(shell pwd)
COMMON_PATH = ${CUR_PATH}/../common

SRC = ${wildcard  ${CUR_PATH}/*.c}

OBJS = ${patsubst %.c, %.o, ${SRC}}
TARGET=test_g
CC=gcc
CCFLAGS=-g -Wall
INC_DIR=
LIB_DIR=

LIBS= -lavdevice -lavfilter -lavformat \
	-lavcodec -lpostproc -lswresample \
	-lswscale -lavutil -lSDL2

RM=rm -f

${TARGET}: ${OBJS}
	${CC} ${OBJS} -o $@  ${LIB_DIR} $(LIBS)

$(OBJS):%.o:%.c
	${CC} ${CCFLAGS} ${INC_DIR} -c $< -o $@

.PHONY:clean
clean:
	$(RM) $(OBJS) ${TARGET}