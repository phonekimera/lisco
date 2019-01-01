dnl -*- m4 -*-
dnl (w) Guido Flohr
dnl $Id: configure.in,v 1.4 2001/09/10 00:10:25 guido Exp $
dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.63])
AC_INIT([tate], [0.5.0])
AC_CONFIG_SRCDIR([src/tate.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([1.16.1 silent-rules parallel-tests])
AM_SILENT_RULES([yes])
 
AC_CONFIG_HEADERS([config.h])

AC_PROG_CPP
AC_PROG_CC
gl_EARLY
gl_INIT
LT_INIT

AC_LANG([C])

AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

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

AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([sys/timeb.h])

AC_FUNC_VPRINTF
AC_CHECK_FUNCS([ftime gettimeofday memcpy])
AC_REPLACE_FUNCS([memset strcasecmp strncasecmp])
if test $am_cv_func_working_getline != yes; then
  AC_CHECK_FUNCS(getdelim)
fi
AC_TYPE_SIGNAL

AC_CONFIG_FILES([Makefile lib/Makefile libchi/libchi.h libchi/Makefile \
                 src/Makefile])
AC_OUTPUT
