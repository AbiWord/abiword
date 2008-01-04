
applix_pkgs="$gsf_req"

APPLIX_CFLAGS=
APPLIX_LIBS=

if test "$enable_applix" == "yes"; then

PKG_CHECK_MODULES(APPLIX,[ $applix_pkgs ])

APPLIX_CFLAGS="$APPLIX_CFLAGS "'${WP_CPPFLAGS}'
APPLIX_LIBS="$APPLIX_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([APPLIX_CFLAGS])
AC_SUBST([APPLIX_LIBS])

