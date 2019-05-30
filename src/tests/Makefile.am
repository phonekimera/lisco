## This file is part of the chess engine tate.
##
## Copyright (C) 2002-2019 cantanea EOOD.
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

noinst_PROGRAMS = check_common check_tateplay

TESTS = ./check_common ./check_tateplay
if HAS_PERL
        TESTS += ./check_mates
endif

check_common_SOURCES = \
		../util.c \
		test_util.c \
		check_common.c

check_tateplay_SOURCES = \
		../tateplay-option.c \
		../tateplay-time-control.c \
		../util.c \
		../xboard-feature.c \
		test_tate_time_control.c \
		test_xboard.c \
		test_uci.c \
		check_tateplay.c

check_common_CFLAGS = $(CFLAGS) $(CHECK_CFLAGS)
check_common_LDADD = ../../lib/libtate.la ../../libchi/libchi.la \
	$(LDFLAGS) $(CHECK_LIBS)
check_tateplay_CFLAGS = $(CFLAGS) $(CHECK_CFLAGS)
check_tateplay_LDADD = ../../lib/libtate.la ../../libchi/libchi.la \
	$(LDFLAGS) $(CHECK_LIBS)

AM_CPPFLAGS = -I.. -I$(srcdir)/.. -I$(top_srcdir)/lib \
	-I$(top_srcdir)/libchi $(AM_CFLAGS)
