OUT=scan

all:
	gcc scan.c scope.c utils.c -o $(OUT) -g