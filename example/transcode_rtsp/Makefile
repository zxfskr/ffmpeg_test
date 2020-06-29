include ../../Makefile
CUR_PATH = $(shell pwd)

SRC = ${wildcard  ${CUR_PATH}/*.c}

OBJS = ${patsubst %.c, %.o, ${SRC}}
TARGET=test_g
CC=gcc
CCFLAGS=-g -Wall ${INC_DIR}
INC_DIR=-I$(COMMON_PATH)
LIBS=-lavdevice -lavfilter -lavformat \
	-lavcodec -lpostproc -lswresample \
	-lswscale -lavutil -lSDL2
RM=rm -f

${TARGET}: ${OBJS}
	${CC} ${OBJS} -o $@ $(LIBS)

$(OBJS):%.o:%.c
	${CC} ${CCFLAGS} -c $< -o $@

.PHONY:clean
clean:
	$(RM) $(OBJS) ${TARGET}
