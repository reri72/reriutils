#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([reriutils], [1.0], [reri0702@gmail.com])
AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_SRCDIR([include/socks.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_ARG_WITH([openssl],
  AS_HELP_STRING([--with-openssl], [Use OpenSSL (yes/no)]),
  [with_openssl=$withval],
  [with_openssl=no])

if test "$with_openssl" = "yes"; then
  LDFLAGS="$LDFLAGS -lssl -lcrypto"
  AC_DEFINE([HAVE_OPENSSL], [1], [Define if OpenSSL is available])
fi

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([bzero getcwd inet_ntoa localtime_r memmove memset mkdir select socket strchr strdup])

AC_CONFIG_FILES([
                    Makefile
                    tests/Makefile
                    tests/conf/Makefile
                    tests/icmp/Makefile
                    tests/if/Makefile
                    tests/log/Makefile
                    tests/raw/Makefile
                    tests/tcp/Makefile
                    tests/udp/Makefile
                    tests/ssl/Makefile
                ])
AC_OUTPUT