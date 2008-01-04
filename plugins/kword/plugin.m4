
kword_pkgs="$gsf_req"

KWORD_CFLAGS=
KWORD_LIBS=

if test "$enable_kword" == "yes"; then

PKG_CHECK_MODULES(KWORD,[ $kword_pkgs ])

KWORD_CFLAGS="$KWORD_CFLAGS "'${WP_CPPFLAGS}'
KWORD_LIBS="$KWORD_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([KWORD_CFLAGS])
AC_SUBST([KWORD_LIBS])

