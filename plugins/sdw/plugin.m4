
sdw_pkgs="$gsf_req"

SDW_CFLAGS=
SDW_LIBS=

if test "$enable_sdw" == "yes"; then

PKG_CHECK_MODULES(SDW,[ $sdw_pkgs ])

SDW_CFLAGS="$SDW_CFLAGS"'${WP_CPPFLAGS}'
SDW_LIBS="$SDW_LIBS"'${PLUGIN_LIBS}'

fi

AC_SUBST([SDW_CFLAGS])
AC_SUBST([SDW_LIBS])

