
wordperfect_pkgs="libwpd-0.8 >= 0.8.0 $gsf_req"
wordperfect_wps_pkgs='libwps-0.1 >= 0.1.0'

WORDPERFECT_CFLAGS=
WORDPERFECT_LIBS=

if test "$enable_wordperfect" == "yes"; then

deps_pkgs="$wordperfect_pkgs"

PKG_CHECK_EXISTS([ $wordperfect_wps_pkgs ],
[
	deps_pkgs="$deps_pkgs $wordperfect_wps_pkgs"
])

PKG_CHECK_MODULES(WORDPERFECT,[ $deps_pkgs ])

WORDPERFECT_CFLAGS="$WORDPERFECT_CFLAGS "'${PLUGIN_CFLAGS}'
WORDPERFECT_LIBS="$WORDPERFECT_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([WORDPERFECT_CFLAGS])
AC_SUBST([WORDPERFECT_LIBS])

