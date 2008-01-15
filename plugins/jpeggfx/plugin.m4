
JPEGGFX_CFLAGS=
JPEGGFX_LIBS=

if test "$enable_jpeggfx" == "yes"; then

AC_MSG_CHECKING([for win32 toolkit])
if test "$TOOLKIT" == "win"; then
	AC_MSG_RESULT([ok])
else
	AC_MSG_ERROR([jpeggfx plugin: only supported on win32])
fi

AC_CHECK_HEADER(jpeglib.h,[
	AC_CHECK_LIB(jpeg,jpeg_read_header,,
	[	
		AC_MSG_ERROR([jpeggfx: libjpeg not found])
	])
],[	
	AC_MSG_ERROR([jpeggfx: jpeg header not found])
])

JPEGGFX_CFLAGS='${PLUGIN_CFLAGS}'
JPEGGFX_LIBS="-ljpeg "'${PLUGIN_LIBS}'

fi

AC_SUBST([JPEGGFX_CFLAGS])
AC_SUBST([JPEGGFX_LIBS])

