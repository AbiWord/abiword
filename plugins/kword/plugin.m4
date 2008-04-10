
KWORD_CFLAGS=
KWORD_LIBS=

if test "$enable_kword" == "yes"; then

KWORD_CFLAGS="$KWORD_CFLAGS "'${PLUGIN_CFLAGS}'
KWORD_LIBS="$KWORD_LIBS "'${PLUGIN_LIBS}'

if test "$enable_kword_builtin" == "yes"; then
	KWORD_CFLAGS="$KWORD_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([KWORD_CFLAGS])
AC_SUBST([KWORD_LIBS])

