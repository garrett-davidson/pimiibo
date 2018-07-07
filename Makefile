SOURCES = amiibo.cpp amiitool.cpp nfchandler.cpp pimiibo.cpp

all: pimiibo amiitoolsubmodule

pimiibo: $(SOURCES)
	$(CXX) $(CXXFlags) $(SOURCES) -o pimiibo -lnfc

amiitoolsubmodule:
	cd amiitool && $(MAKE) amiitool

clean:
	rm pimiibo
	cd amiitool && $(MAKE) clean
