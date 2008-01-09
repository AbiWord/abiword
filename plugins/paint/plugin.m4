
PAINT_CFLAGS=
PAINT_LIBS=

if test "$enable_paint" == "yes"; then

# TODO check for libpng
if test "$TOOLKIT" == "win"; then
	PAINT_LIBS="-lgdi32 -lpng13"
fi

PAINT_CFLAGS="$PAINT_CFLAGS "'${PLUGIN_CFLAGS}'
PAINT_LIBS="$PAINT_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([PAINT_CFLAGS])
AC_SUBST([PAINT_LIBS])

