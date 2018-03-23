all: pimiibo amiitool

pimiibo: test.c
	gcc test.c -o test -lnfc

amiitool:
	@$(MAKE) -C amiitool
