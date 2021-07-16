## This file is part of the chess engine tate.
##
## Copyright (C) 2002-2021 cantanea EOOD.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnits

SUBDIRS = . tests

lib_LTLIBRARIES = libchi.la
libchi_la_SOURCES = libchi.h dump_pieces.c strerror.c make_move.c \
	print_move.c movegen.c clear_position.c \
	coords2label.c init_position.c piece2char.c \
	shift2label.c parse_move.c apply_move.c legal_moves.c \
	check_legality.c illegal_move.c closest_prime.c \
	zk_init.c zk_finish.c zk_signature.c zk_update_signature.c \
	zk_change_side.c update_material.c set_position.c \
	parse_fen_position.c parse_epd.c \
	char2figurine.c game_over.c fen.c stringbuf.c \
	unapply_move.c unmake_move.c coordinate_notation.c free.c \
	magicmoves.c mm_init.c

libchi_la_LDFLAGS = -version-info 0:0:0

libchi_la_LDFLAGS += -no-undefined

AM_CPPFLAGS = -I. -I$(srcdir) -I.. -I$(top_srcdir)/lib

noinst_PROGRAMS = genmasks showfen
# *Never* add -lchi here (bootstrapping problem).

noinst_HEADERS = stringbuf.h

showfen_LDADD = -lchi $(top_srcdir)/lib/libtate.la
showfen_DEPENDENCIES = $(srcdir)/libchi.la $(top_srcdir)/lib/libtate.la

bitmasks.h: genmasks
	./genmasks >$@.tmp
	$(SHELL) $(srcdir)/../move-if-change $@.tmp $@
	touch $@

movegen.lo: bitmasks.h

DISTCLEANFILES = bitmasks.h

EXTRA_DIST = libchi.h.in movegen_color.c magicmoves.h

