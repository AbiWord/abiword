
WIN32GFX_CFLAGS=
WIN32GFX_LIBS=
win32gfx_deps="no"

if test "$enable_win32gfx" != ""; then

AC_MSG_CHECKING([for win32 toolkit])
if test "$TOOLKIT" == "win"; then
  AC_MSG_RESULT([yes])
  win32gfx_deps="yes"
else
  AC_MSG_RESULT([no])
  if test "$enable_win32gfx" == "auto"; then
    AC_MSG_WARN([win32gfx plugin: only supported on win32])
  else
    AC_MSG_ERROR([win32gfx plugin: only supported on win32])
  fi
fi

fi

if test "$enable_win32gfx" == "yes" || \
   test "$win32gfx_deps" == "yes"; then

PLUGINS="$PLUGINS win32gfx"

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

