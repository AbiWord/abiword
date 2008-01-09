
BMP_CFLAGS=
BMP_LIBS=

if test "$enable_bmp" == "yes"; then

# TODO check for libpng

BMP_CFLAGS="$BMP_CFLAGS "'${PLUGIN_CFLAGS}'
BMP_LIBS="$BMP_LIBS "'${PLUGIN_LIBS} -lpng13'

fi

AC_SUBST([BMP_CFLAGS])
AC_SUBST([BMP_LIBS])

