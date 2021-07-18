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

noinst_PROGRAMS = check_common check_lisco check_liscoplay

TESTS = ./check_common \
	./check_lisco \
	./check_liscoplay \
	./check_mates \
	./check_dm1 \
	./check_dm2 \
	./check_dm3

check_common_SOURCES = \
		../util.c \
		test_util.c \
		check_common.c

check_liscoplay_SOURCES = \
		../liscoplay-option.c \
		../liscoplay-time-control.c \
		../util.c \
		../xboard-feature.c \
		test_lisco_time_control.c \
		test_xboard.c \
		test_uci.c \
		check_liscoplay.c

check_lisco_SOURCES = \
		../move-selector.c \
		../perft.c \
		../rtime.c \
		../state.c \
		../think.c \
		../transposition-table.c \
		../uci-engine.c \
		../util.c \
		test_move_selector.c \
		test_perft.c \
		test_transposition_table.c \
		test_uci_engine.c \
		check_lisco.c

check_common_CFLAGS = $(CFLAGS) $(CHECK_CFLAGS)
check_common_LDADD = ../../lib/liblisco.la ../../libchi/libchi.la \
	$(LDFLAGS) $(CHECK_LIBS)
check_lisco_CFLAGS = $(CFLAGS) $(CHECK_CFLAGS)
check_lisco_LDADD = ../../lib/liblisco.la ../../libchi/libchi.la \
	$(LDFLAGS) $(CHECK_LIBS)
check_liscoplay_CFLAGS = $(CFLAGS) $(CHECK_CFLAGS)
check_liscoplay_LDADD = ../../lib/liblisco.la ../../libchi/libchi.la \
	$(LDFLAGS) $(CHECK_LIBS)

AM_CPPFLAGS = -I.. -I$(srcdir)/.. -I$(top_srcdir)/lib \
	-I$(top_srcdir)/libchi $(AM_CFLAGS)
