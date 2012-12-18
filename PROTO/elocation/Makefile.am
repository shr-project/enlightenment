ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

MAINTAINERCLEANFILES = \
ABOUT-NLS \
Makefile.in \
aclocal.m4 \
compile \
config.cache-env \
config.guess \
config.h.in \
config.h.in~ \
config.rpath \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
missing \
mkinstalldirs \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.gz \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 \
$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc.tar.bz2 \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4

EXTRA_DIST = \
AUTHORS \
COPYING \
README \
autogen.sh \
elocation.pc.in \
m4/ac_attribute.m4 \
m4/efl_doxygen.m4 \
m4/efl_path_max.m4 \
m4/efl_threads.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = elocation.pc

.PHONY: doc coverage benchmark

# Documentation
doc:
	@echo "entering doc/"
	make -C doc doc

# Unit tests

if EFL_ENABLE_TESTS

lcov-reset:
	@rm -rf $(top_builddir)/coverage
	@find $(top_builddir) -name "*.gcda" -delete
	@lcov --zerocounters --directory $(top_builddir)

lcov-report:
	@mkdir $(top_builddir)/coverage
	lcov --capture --compat-libtool --output-file $(top_builddir)/coverage/coverage.info --directory $(top_builddir)
	lcov --remove $(top_builddir)/coverage/coverage.info '*.h' --output-file $(top_builddir)/coverage/coverage.cleaned.info
	genhtml -t "$(PACKAGE_STRING)" -o $(top_builddir)/coverage/html $(top_builddir)/coverage/coverage.cleaned.info
	@echo "Coverage Report at $(top_builddir)/coverage/html"

check-local:
	@$(MAKE) $(AM_MAKEFLAGS) lcov-reset
	@./src/tests/elocation_suite
	@$(MAKE) $(AM_MAKEFLAGS) lcov-report

else

lcov-reset:
	@echo "reconfigure with --enable-tests"

lcov-report:
	@echo "reconfigure with --enable-tests"

check-local:
	@echo "reconfigure with --enable-tests"

endif

clean-local:
	@rm -rf coverage
