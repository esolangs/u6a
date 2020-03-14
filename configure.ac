AC_PREREQ([2.69])
AC_INIT([u6a], [0.01], [bug-report@cismon.net])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/u6a.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])

dnl Check for operating system
AC_CANONICAL_HOST
case "${host_os}" in
    linux*)
        AC_SUBST([MANPATH_CONF],  [/etc/manpath.config])
        AC_SUBST([REBUILD_MANDB], [mandb])
        AC_SUBST([BASE64_FLAGS],  [-w0])
        ;;
    darwin*)
        AC_SUBST([MANPATH_CONF],  [/private/etc/man.conf])
        AC_SUBST([REBUILD_MANDB], [/usr/libexec/makewhatis])
        AC_SUBST([BASE64_FLAGS],  [-b0])
        ;;
    *)
        AC_MSG_ERROR(["Unsupported OS: ${host_os}"])
        ;;
esac

dnl Checks for programs.
AC_PROG_CC_C99

dnl Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h inttypes.h stddef.h stdint.h stdlib.h string.h unistd.h],
                 [],
                 [AC_MSG_ERROR(["required header(s) not found"])])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getopt_long strtoul])

AC_OUTPUT
