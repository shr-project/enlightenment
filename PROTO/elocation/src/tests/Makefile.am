MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-DTESTS_SRC_DIR=\"$(top_srcdir)/src/tests\" \
@CHECK_CFLAGS@ \
@ELOCATION_CFLAGS@

if EFL_ENABLE_TESTS

check_PROGRAMS = elocation_suite

elocation_suite_SOURCES = elocation_suite.c
elocation_suite_LDADD = $(top_builddir)/src/lib/libelocation.la @ELOCATION_LIBS@ @CHECK_LIBS@

endif
