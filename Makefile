all: pimiibo amiitool

pimiibo: pimiibo.c
	gcc pimiibo.c -o pimiibo -lnfc

amiitool:
	@$(MAKE) -C amiitool
