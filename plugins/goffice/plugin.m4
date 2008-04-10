
goffice_pkgs="$goffice_req"

GOFFICE_CFLAGS=
GOFFICE_LIBS=

if test "$enable_goffice" == "yes"; then

if test "$enable_goffice_builtin" == "yes"; then
AC_MSG_ERROR([static linking is not supported for the `goffice' plugin])
fi

AC_MSG_CHECKING([for gtk toolkit])
if test "$TOOLKIT" == "gtk"; then
  AC_MSG_RESULT([ok])
else
  AC_MSG_ERROR([goffice plugin: only supported with gtk])
fi

PKG_CHECK_MODULES(GOFFICE,[ $goffice_pkgs ])

GOFFICE_CFLAGS="$GOFFICE_CFLAGS "'${PLUGIN_CFLAGS}'
GOFFICE_LIBS="$GOFFICE_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([GOFFICE_CFLAGS])
AC_SUBST([GOFFICE_LIBS])

