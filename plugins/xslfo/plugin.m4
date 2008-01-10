
xslfo_pkgs="$gsf_req"

XSLFO_CFLAGS=
XSLFO_LIBS=

if test "$enable_xslfo" == "yes"; then

PKG_CHECK_MODULES(XSLFO,[ $xslfo_pkgs ])

XSLFO_CFLAGS="$XSLFO_CFLAGS "'${PLUGIN_CFLAGS}'
XSLFO_LIBS="$XSLFO_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([XSLFO_CFLAGS])
AC_SUBST([XSLFO_LIBS])

