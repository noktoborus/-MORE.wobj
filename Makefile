CC=colorgcc
BIN=./file
SRC=src/main.c src/wavefront.c
all:
	@${CC} -o ${BIN} -Wall -Werror -pedantic -std=c99 -g -lGL -lglut ${SRC}
	@${BIN}

