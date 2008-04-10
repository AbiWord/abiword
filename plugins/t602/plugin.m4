
T602_CFLAGS=
T602_LIBS=

if test "$enable_t602" == "yes"; then

T602_CFLAGS="$T602_CFLAGS "'${PLUGIN_CFLAGS}'
T602_LIBS="$T602_LIBS "'${PLUGIN_LIBS}'

if test "$enable_t602_builtin" == "yes"; then
	T602_CFLAGS="$T602_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([T602_CFLAGS])
AC_SUBST([T602_LIBS])

