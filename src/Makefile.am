## Process this file with automake to produce Makefile.in
# (w) Guido Flohr
## $Id: Makefile.am,v 1.4 2001/09/10 12:10:24 guido Exp $
# Process Makefile.in with autoconf to produce Makefile

AUTOMAKE_OPTIONS = gnits

CFLAGS = @CFLAGS@ -Wall

bin_PROGRAMS = tate 

noinst_HEADERS = board.h brain.h tate.h edit.h book.h

tate_SOURCES = tate.c board.c brain.c edit.c book.c
tate_LDADD = ../lib/libutil.a
tate_DEPENDENCIES = $(top_srcdir)/libchi/libchi.la

INCLUDES = -I. -I$(srcdir) -I.. -I$(top_srcdir)/lib -I../lib \
	-I$(top_srcdir)/libchi -I../libchi
LIBS = -lgdbm -lchi
LDADD = ../lib/libutil.a ../libchi/libchi.la
LDFLAGS = -L$(top_srcdir)/libchi -L../libchi

