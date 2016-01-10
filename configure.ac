#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([yfrogupload], [1.0], [/dev/null])
AC_CONFIG_HEADERS([config.h])

# for FreeBSD
CFLAGS="$CFLAGS -I/usr/local/include"
CPPFLAGS="$CPPFLAGS -I/usr/local/include"
LDFLAGS="$LDFLAGS -L/usr/local/lib"

AC_CHECK_HEADER([curl/curl.h], [], [
  AC_MSG_ERROR([Unable to find curl/curl.h])
])

AC_CHECK_LIB([curl], [curl_version], [], [
  AC_MSG_ERROR([Unable to find libcurl])
])

# Checks for programs.
AC_PROG_CC
AM_INIT_AUTOMAKE

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
