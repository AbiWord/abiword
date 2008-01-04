
WIN32GFX_CFLAGS=
WIN32GFX_LIBS=

if test "$enable_win32gfx" == "yes"; then

# TODO get this from configure.in somehow
SYSTEM_LIBS="-lkernel32 -luser32 -lgdi32 -lcomdlg32 -ladvapi32 -lshell32 -luuid -lcomctl32 -lole32 -loleaut32"

WIN32GFX_CFLAGS="$WIN32GFX_CFLAGS "'${WP_CPPFLAGS}'
WIN32GFX_LIBS="$WIN32GFX_LIBS $SYSTEM_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([WIN32GFX_CFLAGS])
AC_SUBST([WIN32GFX_LIBS])

