
wml_pkgs="$gsf_req"

WML_CFLAGS=
WML_LIBS=

if test "$enable_wml" == "yes"; then

PKG_CHECK_MODULES(WML,[ $wml_pkgs ])

WML_CFLAGS="$WML_CFLAGS "'${WP_CPPFLAGS}'
WML_LIBS="$WML_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([WML_CFLAGS])
AC_SUBST([WML_LIBS])

