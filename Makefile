OUT=scan

all:
	gcc scan.c parse.c utils.c list.c -o $(OUT) -g