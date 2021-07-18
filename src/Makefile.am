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

SUBDIRS = . tests

bin_PROGRAMS = lisco liscoplay

noinst_HEADERS = \
	basename.h \
	board.h \
	book.h \
	brain.h \
	log.h \
	perft.h \
	rtime.h \
	search.h \
	lisco.h \
	liscoplay-engine.h \
	liscoplay-game.h \
	liscoplay-loop.h \
	liscoplay-option.h \
	liscoplay-time-control.h \
	think.h \
	time-control.h \
	time-management.h \
	uci-engine.h \
	util.h \
	xboard-feature.h \
	xmalloca-debug.h

lisco_SOURCES = lisco.c uci-engine.c util.c think.c state.c perft.c \
	rtime.c transposition-table.c move-selector.c

# FIXME! Remove liscoplay. Can be replaced with cutechess-cli.
liscoplay_SOURCES = liscoplay.c liscoplay-engine.c log.c liscoplay-game.c \
	liscoplay-loop.c util.c liscoplay-option.c xboard-feature.c display-board.c \
	liscoplay-time-control.c

lisco_LDADD = ../lib/liblisco.la ../libchi/libchi.la
lisco_DEPENDENCIES = $(top_srcdir)/libchi/libchi.la
liscoplay_LDADD = ../lib/liblisco.la ../libchi/libchi.la
liscoplay_DEPENDENCIES = $(top_srcdir)/libchi/libchi.la

AM_CPPFLAGS = -I. -I$(srcdir) -I.. -I$(top_srcdir)/lib -I../lib \
	-I$(top_srcdir)/libchi -I../libchi $(AM_CFLAGS)
LIBS = -lgdbm
