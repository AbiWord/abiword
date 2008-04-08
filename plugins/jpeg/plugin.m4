
JPEG_CFLAGS=
JPEG_LIBS=

if test "$enable_jpeg" == "yes"; then

AC_MSG_CHECKING([for win32 toolkit])
if test "$TOOLKIT" == "win"; then
	AC_MSG_RESULT([ok])
else
	AC_MSG_ERROR([jpeg plugin: only supported on win32])
fi

AC_CHECK_HEADER(jpeglib.h,[
	AC_CHECK_LIB(jpeg,jpeg_read_header,,
	[	
		AC_MSG_ERROR([jpeg: libjpeg not found])
	])
],[	
	AC_MSG_ERROR([jpeg: jpeg header not found])
])

JPEG_CFLAGS='${PLUGIN_CFLAGS}'
JPEG_LIBS="-ljpeg "'${PLUGIN_LIBS}'

fi

AC_SUBST([JPEG_CFLAGS])
AC_SUBST([JPEG_LIBS])

