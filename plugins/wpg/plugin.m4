
wpg_pkgs="$gsf_req libwpg-0.1 >= 0.1.0 libwpd-0.8 >= 0.8.0"

WPG_CFLAGS=
WPG_LIBS=

if test "$enable_wpg" == "yes"; then

if test "$enable_wpg_builtin" == "yes"; then
AC_MSG_ERROR([static linking is not supported for the `wpg' plugin])
fi

PKG_CHECK_MODULES(WPG, [ $wpg_pkgs ])

WPG_CFLAGS="$WPG_CFLAGS "'${PLUGIN_CFLAGS}'
WPG_LIBS="$WPG_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([WPG_CFLAGS])
AC_SUBST([WPG_LIBS])

