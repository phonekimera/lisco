## Makefile for the support lib subdirectory of brutus.
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

AUTOMAKE_OPTIONS = 1.5 gnits no-dependencies

CFLAGS = -Wall @CFLAGS@

noinst_LIBRARIES = libutil.a

libutil_a_SOURCES = error.c xmalloc.c xstrdup.c
LIBADD_SOURCE =  \
    getline.c getline.h \
    memset.c \
    strcasecmp.c strncasecmp.c

EXTRA_DIST = $(LIBADD_SOURCE)

noinst_HEADERS = system.h getline.h error.h

