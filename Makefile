OUT=scan

all:
	gcc scan.c parse.c utils.c -o $(OUT) -g