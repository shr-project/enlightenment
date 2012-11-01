MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = gen_script.sh

uninstall-local:
	rm $(DESTDIR)$(bindir)/clouseau
	rm $(DESTDIR)$(bindir)/clouseau_start

clean-local:
	rm clouseau
	rm clouseau_start

install-exec-local:
	$(srcdir)/gen_script.sh $(libdir)/clouseau
	$(INSTALL) clouseau_start $(DESTDIR)$(bindir)
	$(INSTALL) clouseau $(DESTDIR)$(bindir)
