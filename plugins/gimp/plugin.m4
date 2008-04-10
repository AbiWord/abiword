
GIMP_CFLAGS=
GIMP_LIBS=

if test "$enable_gimp" == "yes"; then

GIMP_CFLAGS="$GIMP_CFLAGS "'${PLUGIN_CFLAGS}'
GIMP_LIBS="$GIMP_LIBS "'${PLUGIN_LIBS}'

if test "$enable_gimp_builtin" == "yes"; then
	GIMP_CFLAGS="$GIMP_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([GIMP_CFLAGS])
AC_SUBST([GIMP_LIBS])

