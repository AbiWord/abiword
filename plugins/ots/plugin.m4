
ots_pkgs="$libglade_req libots-1 >= 0.5.0"

OTS_CFLAGS=
OTS_LIBS=

if test "$enable_ots" == "yes"; then

PKG_CHECK_MODULES(OTS,[ $ots_pkgs ])

OTS_CFLAGS="$OTS_CFLAGS"'${WP_CPPFLAGS}'
OTS_LIBS="$OTS_LIBS"'${PLUGIN_LIBS}'

fi

AC_SUBST([OTS_CFLAGS])
AC_SUBST([OTS_LIBS])

