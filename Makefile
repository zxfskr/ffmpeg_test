ROOT_PATH=/home/zxfeng/software/ffmpeg_test

SRC = ${wildcard  ${ROOT_PATH}/*.c} \

OBJS = ${patsubst %.c, %.o, ${SRC}}
TARGET=test_g
CC=gcc
CCFLAGS=-g -Wall
LIBS=-lavformat -lavcodec -lavutil -lavfilter
RM=rm -f

${TARGET}: ${OBJS}
	${CC} ${OBJS} -o $@ $(LIBS)

$(OBJS):%.o:%.c
	${CC} ${CCFLAGS} -c $< -o $@ 

.PHONY:clean
clean:
	$(RM) $(OBJS) ${TARGET}

