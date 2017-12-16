dnl Process this file with autoconf to produce a configure script.

dnl init

AC_INIT([malint], [0.2])
AC_CONFIG_SRCDIR([malint.c])
AM_INIT_AUTOMAKE(foreign)
AM_CONFIG_HEADER(config.h)

# We want these before the checks, so the checks can modify their values.
test -z "$CFLAGS" && CFLAGS=-g auto_cflags=1

AC_PROG_CC

dnl # If we're using gcc and the user hasn't specified CFLAGS, add -O to CFLAGS.
dnl test -n "$GCC" && test -n "$auto_cflags" && CFLAGS="$CFLAGS -O"

dnl programs
dnl libraries
dnl header files
dnl typedefs
dnl structures
dnl compiler characteristics

dnl library functions

AC_REPLACE_FUNCS(strerror) dnl asprintf vasprintf
AC_CHECK_FUNCS(getopt_long, , [AC_LIBOBJ(getopt1) AC_LIBOBJ(getopt)])

dnl system services

dnl output
AC_OUTPUT(Makefile regress/Makefile)
