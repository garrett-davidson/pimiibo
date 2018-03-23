all: pimiibo

pimiibo: test.c
	gcc test.c -o test -lnfc
