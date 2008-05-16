
PAINT_CFLAGS=
PAINT_LIBS=

if test "$enable_paint" != ""; then

PLUGINS="$PLUGINS applix"

# TODO check for libpng
if test "$TOOLKIT" == "win"; then
	PAINT_LIBS="-lgdi32 -lpng13"
fi

PAINT_CFLAGS="$PAINT_CFLAGS "'${PLUGIN_CFLAGS}'
PAINT_LIBS="$PAINT_LIBS "'${PLUGIN_LIBS}'

if test "$enable_paint_builtin" == "yes"; then
	PAINT_CFLAGS="$PAINT_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([PAINT_CFLAGS])
AC_SUBST([PAINT_LIBS])

