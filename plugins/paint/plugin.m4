
PAINT_CFLAGS=
PAINT_LIBS=

if test "$enable_paint" != ""; then

test "$enable_paint" == "auto" && PLUGINS="$PLUGINS paint"

# TODO check for libpng
if test "$TOOLKIT" == "win"; then
	PAINT_LIBS="-lgdi32 $PNB_LIBS"
	PAINT_CFLAGS="$PAINT_CFLAGS $PNG_CFLAGS"
fi

PAINT_CFLAGS="$PAINT_CFLAGS "'${PLUGIN_CFLAGS}'
PAINT_LIBS="$PAINT_LIBS "'${PLUGIN_LIBS}'

if test "$enable_paint_builtin" == "yes"; then
	PAINT_CFLAGS="$PAINT_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([PAINT_CFLAGS])
AC_SUBST([PAINT_LIBS])

