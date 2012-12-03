MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) \
	-I$(top_srcdir)/src/bin \
	-I$(top_srcdir)/src/include \
	-DMODULE_BUILD=1 \
	-DPACKAGE_DATA_DIR=\"$(MODULE_DIR)/sawed-off\" \
	@E_CFLAGS@ \
	@SHOTGUN_CFLAGS@

filesdir = $(MODULE_DIR)/sawed-off
files_DATA = module.desktop e-module-sawed-off.edj

pkgdir = $(MODULE_DIR)/sawed-off/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = e_mod_main.h \
		e_mod_main.c \
		e_mod_config.c

module_la_LIBADD = @E_LIBS@ @SHOTGUN_LIBS@
module_la_LDFLAGS = -module -avoid-version

EXTRA_DIST = e-module-sawed-off.edc

EDJE_FLAGS = -v

%.edj: %.edc
	$(edje_cc) $(EDJE_FLAGS) $< $@

clean-local:
	rm -rf *~ e-module-sawed-off.edj
