
wpggfx_pkgs="$gsf_req libwpg-0.1 >= 0.1.0 libwpd-0.8 >= 0.8.0"

WPGGFX_CFLAGS=
WPGGFX_LIBS=

if test "$enable_wpggfx" == "yes"; then

PKG_CHECK_MODULES(WPGGFX, [ $wpggfx_pkgs ])

WPG_CFLAGS="$WPGGFX_CFLAGS "'${PLUGIN_CFLAGS}'
WPG_LIBS="$WPGGFX_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([WPGGFX_CFLAGS])
AC_SUBST([WPGGFX_LIBS])

