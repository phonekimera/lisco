## This file is part of the chess engine lisco.
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

noinst_PROGRAMS = check_libchi

TESTS = ./check_libchi

check_libchi_SOURCES = \
		test_see.c \
		test_coordinate_notation.c \
		test_fen.c \
		test_game_over.c \
		test_legal_moves.c \
		test_move_making.c \
		test_move_making_pgn.c \
		test_parsers.c \
		test_presentation.c \
		check_libchi.c

check_libchi_CFLAGS = $(CFLAGS) $(CHECK_CFLAGS)
check_libchi_LDADD = ../../lib/liblisco.la ../libchi.la $(LDFLAGS) $(CHECK_LIBS)

AM_CPPFLAGS = -I.. -I$(srcdir)/.. -I$(top_srcdir)/lib \
	-I$(top_srcdir)/libchi $(AM_CFLAGS)
