.PHONY: default

all: default

# XXX shouldn't be phony, but managing all dependence edc is a pain
default:
	edje_cc -id images default.edc -o b_and_w.edj

install: default
	cp b_and_w.edj ~/.e/e/themes
