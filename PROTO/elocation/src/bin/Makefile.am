MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
@EFL_ELOCATION_BUILD@ \
-I$(top_srcdir) \
-I$(top_srcdir)/bin \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@ELOCATION_CFLAGS@

bin_PROGRAMS = elocation-test
elocation_test_SOURCES = elocation-test.c

elocation_test_CFLAGS = @ELOCATION_CFLAGS@
elocation_test_LDADD = $(top_builddir)/src/lib/libelocation.la @ELOCATION_LIBS@

elocation_test_LDFLAGS = -export-dynamic
