
BMP_CFLAGS=
BMP_LIBS=

if test "$enable_bmp" == "yes"; then

AC_MSG_CHECKING([for win32 toolkit])
if test "$TOOLKIT" == "win"; then
	AC_MSG_RESULT([ok])
else
	AC_MSG_ERROR([bmp plugin: only supported on win32])
fi

# TODO check for libpng, well abiword links to it anyways

BMP_CFLAGS="$BMP_CFLAGS "'${PLUGIN_CFLAGS}'
BMP_LIBS="$BMP_LIBS "'${PLUGIN_LIBS} -lpng13'

if test "$enable_bmp_builtin" == "yes"; then
	BMP_CFLAGS="$BMP_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([BMP_CFLAGS])
AC_SUBST([BMP_LIBS])

