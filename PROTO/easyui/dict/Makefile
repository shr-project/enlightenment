URL="http://www.gutenberg.org/files/29765/29765-8.zip"
PACKAGE=parse

CC=gcc
CFLAGS:=$(shell pkg-config --cflags --libs eet eina)

dict.eet: parse
	./$(PACKAGE) dict.eet

parse: 29765-8.txt
	python $(PACKAGE).py . > $(PACKAGE).c
	$(CC) -o $(PACKAGE) $(PACKAGE).c $(CFLAGS)

29765-8.txt: 29765-8.zip
	@[ -f 29765-8.txt ] || unzip -oq 29765-8.zip

29765-8.zip:
	wget -c --user-agent="Mozilla/5.0 (MSIE 6.0; compatible)" $(URL)

clean:
	rm -f dict.eet
	rm -f parse
	rm -f parse.c
	rm -f 29765-8.txt
	rm -f 29765-8.zip
