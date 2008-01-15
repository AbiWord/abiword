
mathview_pkgs='mathview-frontend-libxml2 >= 0.7.5'

MATHVIEW_CFLAGS=
MATHVIEW_LIBS=

# need to unconditionally test, for `make distcheck'
AM_PROG_LEX
AC_PROG_YACC

if test "$enable_mathview" == "yes"; then

PKG_CHECK_MODULES(MATHVIEW,[ $mathview_pkgs ])

MATHVIEW_CFLAGS="$MATHVIEW_CFLAGS "'${PLUGIN_CFLAGS}'
MATHVIEW_LIBS="$MATHVIEW_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([MATHVIEW_CFLAGS])
AC_SUBST([MATHVIEW_LIBS])

