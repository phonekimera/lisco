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

noinst_PROGRAMS = check_libchi

TESTS = ./check_libchi

check_libchi_SOURCES = \
        check_libchi.c

check_libchi_CFLAGS = $(CFLAGS) $(CHECK_CFLAGS)
check_libchi_LDADD = ../libchi.la $(LDFLAGS) $(CHECK_LIBS)

AM_CPPFLAGS = -I.. -I$(srcdir)/.. -I$(top_srcdir)/libchi $(AM_CFLAGS)
