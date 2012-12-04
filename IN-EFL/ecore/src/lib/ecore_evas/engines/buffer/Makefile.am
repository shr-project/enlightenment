MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_file \
-I$(top_srcdir)/src/lib/ecore_ipc \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_input_evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_file \
-I$(top_builddir)/src/lib/ecore_ipc \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_input_evas \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@EFL_ECORE_EVAS_BUILD@ \
$(ECORE_IPC_INC) \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

pkgdir = $(libdir)/ecore_evas/engines/buffer/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = ecore_evas_buffer.c

if BUILD_ECORE_EVAS_EXTN
module_la_SOURCES += ecore_evas_extn.c
endif

module_la_LIBADD = \
$(ECORE_IPC_LIB) \
$(top_builddir)/src/lib/ecore_file/libecore_file.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_input_evas/libecore_input_evas.la \
$(top_builddir)/src/lib/ecore/libecore.la \
@EVAS_LIBS@ \
@EINA_LIBS@ \
@EFL_SHM_OPEN_LIBS@

module_la_LDFLAGS = -module -no-undefined -version-info @version_info@ @release_info@
module_la_LIBTOOLFLAGS = --tag=disable-static

EXTRA_DIST = \
ecore_evas_buffer_private.h
