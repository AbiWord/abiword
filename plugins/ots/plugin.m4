
ots_pkgs="$libglade_req libots-1 >= 0.5.0"

OTS_CFLAGS=
OTS_LIBS=

if test "$enable_ots" == "yes"; then

if test "$enable_ots_builtin" == "yes"; then
AC_MSG_ERROR([static linking is not supported for the `ots' plugin])
fi

PKG_CHECK_MODULES(OTS,[ $ots_pkgs ])

OTS_CFLAGS="$OTS_CFLAGS "'${PLUGIN_CFLAGS}'
OTS_LIBS="$OTS_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([OTS_CFLAGS])
AC_SUBST([OTS_LIBS])

