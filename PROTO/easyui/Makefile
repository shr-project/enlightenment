
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

GUIDESDIR = jsduck/guides
READMES = $(EXAMPLES:%.js=$(GUIDESDIR)/%/README.md)

docs: $(READMES)
	jsduck --config=./jsduck/config.json

$(GUIDESDIR)/%/README.md: %.js
	mkdir -p $(@D)
	sed 's/^/\t/' < $< > $@

clean:
	rm -f $(READMES)
