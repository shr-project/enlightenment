
SOURCES = \
  eui.js
  
EXAMPLES = \
	browser.js \
	calculator.js \
	dict.js \
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
	themes/default.edc \
	themes/dict.edc

GUIDESDIR = jsduck/guides
ELEMENTARY_THEMES_DIR = \
	../e/trunk/elementary/data/themes \
	themes/
ELEMENTARY_FONTS_DIR = \
	themes/fonts

READMES = $(EXAMPLES:%.js=$(GUIDESDIR)/%/README.md)
EDJE_CC_ID = $(ELEMENTARY_THEMES_DIR:%=-id %)
EDJE_CC_FD = $(ELEMENTARY_FONTS_DIR:%=-fd %)
EDJS = $(EDCS:%.edc=%.edj)

all: docs edc

docs: $(READMES) $(SOURCES)
	jsduck --config=./jsduck/config.json

$(GUIDESDIR)/%/README.md: %.js
	mkdir -p $(@D)
	sed 's/^/\t/' < $< > $@

%.edj: %.edc
	edje_cc $(EDJE_CC_ID) $(EDJE_CC_FD) $<

edc: $(EDJS)

clean:
	rm -f $(READMES)
	rm -f themes/*.edj
