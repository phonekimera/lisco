dnl This file is part of the chess engine tate.
dnl
dnl Copyright (C) 2002-2021 cantanea EOOD.
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ([2.71])
AC_INIT([tate],[0.6.0])
AC_CONFIG_SRCDIR([src/tate.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([1.16.3 silent-rules parallel-tests subdir-objects])
AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS([config.h])

AC_PROG_CPP
AC_PROG_CC
gl_EARLY
gl_INIT
LT_INIT

AC_CHECK_PROG([PERL], [perl], [yes], [no])
AM_CONDITIONAL([HAS_PERL], [test "x$PERL" = "xyes"])

AC_LANG([C])

AC_CANONICAL_HOST
LT_INIT([win32-dll])

PKG_CHECK_MODULES([CHECK], [check])

AC_DEFINE(_GNU_SOURCE, 1, [Make sure we see all GNU extensions.])

AC_C_CONST
AC_C_BIGENDIAN
AC_C_BACKSLASH_A

AC_CHECK_LIB(m, log10, , AC_MSG_ERROR(unable to find function log10))

AC_CHECK_SIZEOF(unsigned long)
AC_CHECK_SIZEOF(unsigned long long)
if test $ac_cv_sizeof_unsigned_long_long = 8; then
        BITV64="unsigned long long"
else
        if test $ac_cv_sizeof_unsigned_long = 8; then
                BITV64="unsigned long"
        else
                AC_MSG_ERROR(unable to find a 64 bit unsigned type)
        fi
fi

AC_SUBST(BITV64)

AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP

AC_CHECK_HEADERS([sys/timeb.h])

AC_FUNC_VPRINTF
AC_CHECK_FUNCS([ftime gettimeofday memcpy])
AC_REPLACE_FUNCS([memset strcasecmp strncasecmp])
if test $am_cv_func_working_getline != yes; then
  AC_CHECK_FUNCS(getdelim)
fi

AC_CONFIG_FILES([Makefile lib/Makefile
                 libchi/libchi.h libchi/Makefile libchi/tests/Makefile \
                 src/Makefile src/tests/Makefile])
AC_OUTPUT
