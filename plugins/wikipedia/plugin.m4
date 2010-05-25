
WIKIPEDIA_CFLAGS=
WIKIPEDIA_LIBS=

if test "$enable_wikipedia" != ""; then

test "$enable_wikipedia" = "auto" && PLUGINS="$PLUGINS wikipedia"

WIKIPEDIA_CFLAGS="$WIKIPEDIA_CFLAGS "'${PLUGIN_CFLAGS}'
WIKIPEDIA_LIBS="$WIKIPEDIA_LIBS "'${PLUGIN_LIBS}'

if test "$enable_wikipedia_builtin" = "yes"; then
	WIKIPEDIA_CFLAGS="$WIKIPEDIA_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([WIKIPEDIA_CFLAGS])
AC_SUBST([WIKIPEDIA_LIBS])

