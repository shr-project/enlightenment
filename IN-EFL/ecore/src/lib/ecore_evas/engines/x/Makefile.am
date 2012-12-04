MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_file \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_input_evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_input_evas \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@EFL_ECORE_EVAS_BUILD@ \
$(ECORE_IPC_INC) \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@ \
@EVIL_CFLAGS@ \
@WAYLAND_EGL_CFLAGS@

pkgdir = $(libdir)/ecore_evas/engines
pkg_LTLIBRARIES = x.la
x_la_SOURCES = ecore_evas_x.c

x_la_LIBADD = \
$(ECORE_IPC_LIB) \
$(top_builddir)/src/lib/ecore_file/libecore_file.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_input_evas/libecore_input_evas.la \
$(top_builddir)/src/lib/ecore/libecore.la \
@EVAS_LIBS@ \
@EINA_LIBS@ \
@EVIL_LIBS@ \
@EFL_SHM_OPEN_LIBS@

x_la_LDFLAGS = -module -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@
x_la_LIBTOOLFLAGS = --tag=disable-static
