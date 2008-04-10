
DASH_CFLAGS=
DASH_LIBS=

if test "$enable_dash" == "yes"; then

DASH_CFLAGS="$DASH_CFLAGS "'${PLUGIN_CFLAGS}'
DASH_LIBS="$DASH_LIBS "'${PLUGIN_LIBS}'

if test "$enable_dash_builtin" == "yes"; then
	DASH_CFLAGS="$DASH_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([DASH_CFLAGS])
AC_SUBST([DASH_LIBS])

