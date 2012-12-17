AM_CFLAGS = \
  -I@top_srcdir@ \
  @MYSQL_CFLAGS@ \
  $(CC_PIC)


noinst_LTLIBRARIES = libmysac.la
libmysac_la_LIBADD = @MYSQL_LIBS@
libmysac_la_SOURCES = \
mysac.c \
mysac_encode_values.c \
mysac_query.c \
mysac_connect.c \
mysac_encode_values.h \
mysac_res.c \
mysac_decode_field.c \
mysac_errors.c \
mysac_stmt.c \
mysac_decode_field.h \
mysac.h \
mysac_usedatabase.c \
mysac_decode_respbloc.c \
mysac_memory.c \
mysac_utils.c \
mysac_decode_respbloc.h \
mysac_memory.h \
mysac_utils.h \
mysac_decode_row.c \
mysac_net.c \
mysac_decode_row.h \
mysac_net.h


MAINTAINERCLEANFILES = Makefile.in
