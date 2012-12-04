MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_IPC
ECORE_IPC_INC= \
-I$(top_srcdir)/src/lib/ecore_ipc \
-I$(top_builddir)/src/lib/ecore_ipc

ECORE_IPC_LIB=$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la
endif

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
-DEFL_ECORE_EVAS_BUILD \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@EFL_ECORE_EVAS_BUILD@ \
$(ECORE_IPC_INC) \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@ \
@EVIL_CFLAGS@

lib_LTLIBRARIES = libecore_evas.la
includes_HEADERS = Ecore_Evas.h Ecore_Evas_Types.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_evas_la_SOURCES = \
ecore_evas_module.c \
ecore_evas_ews.c \
ecore_evas.c \
ecore_evas_util.c

libecore_evas_la_LIBADD = \
$(ECORE_IPC_LIB) \
$(top_builddir)/src/lib/ecore_file/libecore_file.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_input_evas/libecore_input_evas.la \
$(top_builddir)/src/lib/ecore/libecore.la \
@EVAS_LIBS@ \
@EINA_LIBS@ \
@EVIL_LIBS@ \
@EFL_SHM_OPEN_LIBS@

libecore_evas_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@

EXTRA_DIST = \
ecore_evas_private.h

SUBDIRS = . engines

