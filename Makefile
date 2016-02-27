# Straight forward Makefile to compile all examples in a row

INCDIR=-I./esCommon
LIBS=-lGLESv2 -lEGL -lm -lX11 -lpthread

COMMONSRC=./esCommon/esShader.c    \
          ./esCommon/esTransform.c \
          ./esCommon/esShapes.c    \
          ./esCommon/esUtil.c
COMMONHRD=esUtil.h

SRC=./cRadar.cpp \
    ./ui_draw.c  

default: all

all: ./cRadar 

clean:
	find . -name "cRadar" | xargs rm -f

#./Chapter_2/Hello_Triangle/CH02_HelloTriangle: ${COMMONSRC} ${COMMONHDR} ${CH02SRC}
#	gcc ${COMMONSRC} ${CH02SRC} -o $@ ${INCDIR} ${LIBS}

./cRadar: ${COMMONSRC} ${COMMONHDR} ${SRC}
	gcc ${COMMONSRC} ${SRC} -o $@ ${INCDIR} ${LIBS}