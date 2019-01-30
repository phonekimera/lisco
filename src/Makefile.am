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

SUBDIRS = . tests

bin_PROGRAMS = tate tateplay

noinst_HEADERS = \
	basename.h \
	board.h \
	book.h \
	brain.h \
	edit.h \
	log.h \
	tate.h \
	tateplay-engine.h \
	tateplay-game.h \
	tateplay-loop.h \
	tateplay-option.h \
	time_ctrl.h \
	util.h \
	xboard-feature.h \
	xmalloca-debug.h

tate_SOURCES = tate.c board.c brain.c edit.c book.c time_ctrl.c \
	evaluate.c next_move.c quiescence.c search.c print_pv.c \
	tt_hash.c epd.c ev_hash.c qtt_hash.c three-fold.c
tateplay_SOURCES = tateplay.c tateplay-engine.c log.c tateplay-game.c \
	tateplay-loop.c util.c tateplay-option.c xboard-feature.c display-board.c

tate_LDADD = ../lib/libtate.la ../libchi/libchi.la
tate_DEPENDENCIES = $(top_srcdir)/libchi/libchi.la
tateplay_LDADD = ../lib/libtate.la ../libchi/libchi.la
tateplay_DEPENDENCIES = $(top_srcdir)/libchi/libchi.la

AM_CPPFLAGS = -I. -I$(srcdir) -I.. -I$(top_srcdir)/lib -I../lib \
	-I$(top_srcdir)/libchi -I../libchi $(AM_CFLAGS)
LIBS = -lgdbm
