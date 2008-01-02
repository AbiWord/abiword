
rsvg_pkgs="librsvg-2.0 >= 2.0"

RSVG_CFLAGS=
RSVG_LIBS=

if test "$enable_rsvg" == "yes"; then

PKG_CHECK_MODULES(RSVG,[ $rsvg_pkgs ])

RSVG_CFLAGS="$RSVG_CFLAGS"'${WP_CPPFLAGS}'
RSVG_LIBS="$RSVG_LIBS"'${PLUGIN_LIBS}'

fi

AC_SUBST([RSVG_CFLAGS])
AC_SUBST([RSVG_LIBS])

