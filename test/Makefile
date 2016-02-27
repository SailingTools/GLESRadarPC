# Straight forward Makefile to compile all examples in a row

INCDIR=-I./Common
LIBS=-lGLESv2 -lEGL -lm -lX11

COMMONSRC=./Common/esShader.c    \
          ./Common/esTransform.c \
          ./Common/esShapes.c    \
          ./Common/esUtil.c
COMMONHRD=esUtil.h

CH02SRC=./Hello_Triangle.c

default: all

all: ./CH02_HelloTriangle

clean:
	find . -name "CH??_*" | xargs rm -f

./CH02_HelloTriangle: ${COMMONSRC} ${COMMONHDR} ${CH02SRC}
	gcc ${COMMONSRC} ${CH02SRC} -o $@ ${INCDIR} ${LIBS}

