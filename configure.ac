AC_INIT([libshell], 1.0)
AM_INIT_AUTOMAKE([subdir-objects foreign])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE([enable])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([source/config.h])
AC_CONFIG_FILES([Makefile Doxyfile])

dnl
dnl Library versioning
dnl
AC_SUBST([SH_CURRENT], 0)
AC_SUBST([SH_REVISION], 0)
AC_SUBST([SH_AGE], 0)

AC_PROG_CC
AC_PROG_LIBTOOL

AC_TYPE_SIZE_T

AC_OUTPUT
