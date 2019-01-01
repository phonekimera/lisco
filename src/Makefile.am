## Process this file with automake to produce Makefile.in
# (w) Guido Flohr
## $Id: Makefile.am,v 1.4 2001/09/10 12:10:24 guido Exp $
# Process Makefile.in with autoconf to produce Makefile

AUTOMAKE_OPTIONS = gnits

bin_PROGRAMS = tate 

noinst_HEADERS = board.h brain.h tate.h edit.h book.h time_ctrl.h

tate_SOURCES = tate.c board.c brain.c edit.c book.c time_ctrl.c \
	evaluate.c next_move.c quiescence.c search.c print_pv.c \
	tt_hash.c epd.c ev_hash.c qtt_hash.c

tate_LDADD = ../lib/libtate.la
tate_DEPENDENCIES = $(top_srcdir)/libchi/libchi.la

AM_CPPFLAGS = -I. -I$(srcdir) -I.. -I$(top_srcdir)/lib -I../lib \
	-I$(top_srcdir)/libchi -I../libchi
LIBS = -lgdbm -lchi
LDADD = ../lib/libtate.la ../libchi/libchi.la
AM_LDFLAGS = -L$(top_srcdir)/libchi -L../libchi

