dnl 
dnl Copyright (C) 2020  CismonX <admin@cismon.net>
dnl 
dnl Copying and distribution of this file, with or without modification, are
dnl permitted in any medium without royalty, provided the copyright notice and
dnl this notice are preserved. This file is offered as-is, without any warranty.
dnl 

AC_PREREQ([2.69])
AC_INIT([u6a], [0.1.2], [bug-report@cismon.net])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/u6a.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])

# Check for operating system
AC_CANONICAL_HOST
case "${host_os}" in
    linux*)
        AC_SUBST([BASE64_ENCODE], [-w0])
        AC_SUBST([BASE64_DECODE], [-d])
        ;;
    darwin*)
        AC_SUBST([BASE64_ENCODE], [-b0])
        AC_SUBST([BASE64_DECODE], [-D])
        ;;
esac

# Checks for programs.
AC_PROG_CC_STDC

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h inttypes.h stddef.h stdint.h stdlib.h string.h unistd.h],
                 [],
                 [AC_MSG_ERROR(["required header(s) not found"])])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getopt_long strtoul])

AC_OUTPUT
