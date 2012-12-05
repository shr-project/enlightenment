MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = gen_script.sh

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/clouseau
	rm -f $(DESTDIR)$(bindir)/clouseau_start

clean-local:
	rm -f clouseau
	rm -f clouseau_start

install-exec-local:
	$(srcdir)/gen_script.sh $(libdir)/clouseau
	$(INSTALL) clouseau_start $(DESTDIR)$(bindir)
	$(INSTALL) clouseau $(DESTDIR)$(bindir)
