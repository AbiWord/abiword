
URLDICT_CFLAGS=
URLDICT_LIBS=

if test "$enable_urldict" != ""; then

test "$enable_urldict" = "auto" && PLUGINS="$PLUGINS urldict"

URLDICT_CFLAGS="$URLDICT_CFLAGS "'${PLUGIN_CFLAGS}'
URLDICT_LIBS="$URLDICT_LIBS "'${PLUGIN_LIBS}'

if test "$enable_urldict_builtin" = "yes"; then
	URLDICT_CFLAGS="$URLDICT_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([URLDICT_CFLAGS])
AC_SUBST([URLDICT_LIBS])

