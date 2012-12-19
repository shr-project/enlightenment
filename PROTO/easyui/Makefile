
SOURCES = \
  eui.js
  
EXAMPLES = \
	browser.js \
	calculator.js \
	dict.js \
	easy-connman.js \
	file.js \
	filter.js \
	group.js \
	infinigag.js \
	notes.js \
	multiple-views.js \
	notes.js \
	photo.js \
	reddit.js \
	rotten_tomatoes.js \
	rss.js \
	split.js \
	test-ajax.js \
	test-routing.js \
	toolbar.js \
	twitter.js \
	video.js

EDCS = \
	themes/eui.edc \
	themes/default.edc

GUIDESDIR = jsduck/guides
ELEMENTARY_THEMES_DIR = \
	../e/elementary/data/themes/images \
	../e/trunk/elementary/data/themes/images \
	themes/
ELEMENTARY_FONTS_DIR = \
	themes/fonts

READMES = $(EXAMPLES:%.js=$(GUIDESDIR)/%/README.md)
EDJE_CC_ID = $(ELEMENTARY_THEMES_DIR:%=-id %)
EDJE_CC_FD = $(ELEMENTARY_FONTS_DIR:%=-fd %)
EDJS = $(EDCS:%.edc=%.edj)

JSDUCK:=$(shell which jsduck 2> /dev/null)

all: dict docs edc

dict:
	make -C dict

docs: $(READMES) $(SOURCES)
	@[ -z $(JSDUCK) ] && { \
		echo "EasyUI documentation is generated using JSDuck, but"; \
		echo "unfortunately it is not installed. You may find it"; \
		echo "at https://github.com/senchalabs/jsduck"; \
		exit 1; \
	};
	$(JSDUCK) --config=./jsduck/config.json

$(GUIDESDIR)/%/README.md: %.js
	mkdir -p $(@D)
	sed 's/^/\t/' < $< > $@

%.edj: %.edc
	edje_cc $(EDJE_CC_ID) $(EDJE_CC_FD) $<

edc: $(EDJS)

clean:
	make clean -C dict
	rm -f $(READMES)
	rm -f themes/*.edj

.PHONY: clean dict
