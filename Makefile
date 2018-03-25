all: pimiibo amiitoolsubmodule

pimiibo: pimiibo.c
	gcc pimiibo.c -o pimiibo -lnfc

amiitoolsubmodule:
	cd amiitool && $(MAKE) amiitool

clean:
	rm pimiibo
	cd amiitool && $(MAKE) clean
