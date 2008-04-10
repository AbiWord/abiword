
WIN32GFX_CFLAGS=
WIN32GFX_LIBS=

if test "$enable_win32gfx" == "yes"; then

AC_MSG_CHECKING([for win32 toolkit])
if test "$TOOLKIT" == "win"; then
  AC_MSG_RESULT([ok])
else
  AC_MSG_ERROR([win32gfx plugin: only supported on win32])
fi

# TODO check for libpng
SYSTEM_LIBS="-lkernel32 -luser32 -lgdi32 -lcomdlg32 -ladvapi32 -lshell32 -luuid -lcomctl32 -lole32 -loleaut32 -lpng13"

WIN32GFX_CFLAGS="$WIN32GFX_CFLAGS "'${PLUGIN_CFLAGS}'
WIN32GFX_LIBS="$WIN32GFX_LIBS $SYSTEM_LIBS "'${PLUGIN_LIBS}'

if test "$enable_win32gfx_builtin" == "yes"; then
	WIN32GFX_CFLAGS="$WIN32GFX_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([WIN32GFX_CFLAGS])
AC_SUBST([WIN32GFX_LIBS])

