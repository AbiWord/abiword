
BMP_CFLAGS=
BMP_LIBS=
bmp_deps="no"

if test "$enable_bmp" != ""; then

   bmp_deps="yes"

fi

if test "$enable_bmp" == "yes" || \
   test "$bmp_deps" == "yes"; then

# TODO check for libpng, well abiword links to it anyways

BMP_CFLAGS="$BMP_CFLAGS $PNG_CFLAGS "'${PLUGIN_CFLAGS}'
BMP_LIBS="$BMP_LIBS $PNG_LIBS "'${PLUGIN_LIBS}'

if test "$enable_bmp_builtin" == "yes"; then
	BMP_CFLAGS="$BMP_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

test "$enable_bmp" == "auto" && PLUGINS="$PLUGINS bmp"

fi

AC_SUBST([BMP_CFLAGS])
AC_SUBST([BMP_LIBS])

