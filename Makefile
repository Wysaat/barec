OUT=scan

all:
	gcc scan.c parse.c addr.c value.c gen_code.c utils.c list.c -o $(OUT) -g -m32