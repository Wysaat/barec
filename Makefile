OUT=scan

all:
	gcc scan.c parse.c gencode.c utils.c -o $(OUT) -g -m32