
JPEG_CFLAGS=
JPEG_LIBS=

if test "$enable_jpeg" == "yes"; then

AC_CHECK_HEADER(jpeglib.h,[
	AC_CHECK_LIB(jpeg,jpeg_read_header,,
	[	
		AC_MSG_ERROR([jpeg: error - libjpeg not found])
	])
],[	
	AC_MSG_ERROR([jpeg: error - jpeg header not found])
])

JPEG_CFLAGS='${PLUGIN_CFLAGS}'
JPEG_LIBS="-ljpeg "'${PLUGIN_LIBS}'

fi

AC_SUBST([JPEG_CFLAGS])
AC_SUBST([JPEG_LIBS])

