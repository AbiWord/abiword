
S5_CFLAGS=
S5_LIBS=

if test "$enable_s5" != ""; then

test "$enable_s5" = "auto" && PLUGINS="$PLUGINS s5"

S5_CFLAGS="$S5_CFLAGS "'${PLUGIN_CFLAGS}'
S5_LIBS="$S5_LIBS "'${PLUGIN_LIBS}'

if test "$enable_s5_builtin" = "yes"; then
	S5_CFLAGS="$S5_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([S5_CFLAGS])
AC_SUBST([S5_LIBS])

