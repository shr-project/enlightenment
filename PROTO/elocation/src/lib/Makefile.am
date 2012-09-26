MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @ELOCATION_CFLAGS@ \
@EFL_ELOCATION_BUILD@

lib_LTLIBRARIES = libelocation.la

includes_HEADERS = Elocation.h
includesdir = $(includedir)/elocation-@VMAJ@

libelocation_la_SOURCES = elocation.c

libelocation_la_LIBADD = @ELOCATION_LIBS@
libelocation_la_LDFLAGS  = -version-info @version_info@ @release_info@
