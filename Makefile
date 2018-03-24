all: pimiibo amiitool

pimiibo: test.c
	gcc test.c -o pimiibo -lnfc

amiitool:
	@$(MAKE) -C amiitool
