## Makefile for libchi.
## Copyright (C) 2002 Guido Flohr (guido@imperia.net)
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnits

CFLAGS = -Wall @CFLAGS@

lib_LTLIBRARIES = libchi.la
libchi_la_SOURCES = libchi.h dump_pieces.c strerror.c make_move.c \
	print_move.c movegen.c bitv2shift.c clear_position.c \
	coords2label.c init_position.c piece2char.c set_piece.c \
	shift2label.c parse_move.c
libchi_la_LDFLAGS = -version-info 0:0:0

INCLUDES = -I. -I$(srcdir) -I.. -I$(top_srcdir)/lib

noinst_PROGRAMS = genmasks chitest
# *Never* add -lchi here (bootstrapping problem).

chitest_LDADD = -lchi
chitest_DEPENDENCIES = $(srcdir)/libchi.la

bitmasks.h: genmasks
	./genmasks >$@.tmp
	$(SHELL) $(srcdir)/../move-if-change $@.tmp $@
	touch $@

movegen.lo: bitmasks.h

DISTCLEANFILES = bitmasks.h

EXTRA_DIST = libchi.h.in movegen_color.c

