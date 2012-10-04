CFLAGS += $(shell pkg-config --cflags elementary evas ecore ecore-input)
LIBS += $(shell pkg-config --libs elementary evas ecore ecore-input)
CFLAGS += -Wall -O2 -g

BINARY=etypers

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin

$(BINARY): etypers.o
	$(CC) -o $@ $< $(LIBS)
	edje_cc -id images/bg.jpg -id ./images etypers.edc

edc: etypers.edc
	edje_cc -id images/bg.jpg -id ./images etypers.edc


install: $(BINARY)
	@echo "installation of $(BINARY)"
	@mkdir -p $(BINDIR)
	@install -m 0755 $(BINARY) $(BINDIR)

clean:
	rm -f $(BINARY) etypers.o etypers.edj

