# Straight forward Makefile to compile all examples in a row

#INCDIR=-I./Common
LIBS=-lGLESv2 -lEGL -lm -lX11

#COMMONSRC=./Common/esShader.c    \
#          ./Common/esTransform.c \
#          ./Common/esShapes.c    \
#          ./Common/esUtil.c
#COMMONHRD=esUtil.h

SRC=./cRadar.cpp

default: all

all: ./cRadar

clean:
	find . -name "cRadar" | xargs rm -f

#./Chapter_2/Hello_Triangle/CH02_HelloTriangle: ${COMMONSRC} ${COMMONHDR} ${CH02SRC}
#	gcc ${COMMONSRC} ${CH02SRC} -o $@ ${INCDIR} ${LIBS}

./cRadar: ${SRC} 
	gcc ${SRC} -o $@ ${LIBS}