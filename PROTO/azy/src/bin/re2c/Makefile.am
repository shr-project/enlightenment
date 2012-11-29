# $Id: Makefile.am 863 2008-05-25 14:38:16Z helly $
CC=$(CXX)
noinst_PROGRAMS = re2c
re2c_SOURCES = code.cc dfa.cc main.cc actions.cc substr.cc \
	translate.cc mbo_getopt.cc scanner.cc \
	basics.h dfa.h globals.h ins.h parser.h re_parser.y re.h scanner.h \
	substr.h token.h mbo_getopt.h code.h stream_lc.h code_names.h $(BUILT_SOURCES)
EXTRA_DIST = \
  CHANGELOG \
  re_parser.y \
  re_parser.h \
  re_parser.c


BUILT_SOURCES = re_parser.h

.PHONY: re_parser.h

AM_YFLAGS = -d

DISTCLEANFILES = re_parser.c re_parser.h
CLEANFILES = parser.cc y.tab.h
MAINTAINERCLEANFILES = Makefile.in
