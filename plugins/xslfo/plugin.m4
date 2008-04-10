
XSLFO_CFLAGS=
XSLFO_LIBS=

if test "$enable_xslfo" == "yes"; then

XSLFO_CFLAGS="$XSLFO_CFLAGS "'${PLUGIN_CFLAGS}'
XSLFO_LIBS="$XSLFO_LIBS "'${PLUGIN_LIBS}'

if test "$enable_xslfo_builtin" == "yes"; then
	XSLFO_CFLAGS="$XSLFO_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([XSLFO_CFLAGS])
AC_SUBST([XSLFO_LIBS])

