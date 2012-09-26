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

bin_PROGRAMS = geoclue-test
geoclue_test_SOURCES = geoclue-test.c

geoclue_test_CFLAGS = @ELOCATION_CFLAGS@
#geoclue_test_LDADD = $(top_builddir)/src/lib/libelocation.la @ELOCATION_LIBS@
geoclue_test_LDADD = @ELOCATION_LIBS@
geoclue_test_LDFLAGS = -export-dynamic
