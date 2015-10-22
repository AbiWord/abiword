
wordperfect_pkgs="libwpd-0.10 $gsf_req"
wordperfect_wps_pkgs='libwps-0.4'
wordperfect_deps="no"

WORDPERFECT_CFLAGS=
WORDPERFECT_LIBS=
WPS_DEFINE=

if test "$enable_wordperfect" != ""; then

PKG_CHECK_EXISTS([ $wordperfect_pkgs ], 
[
	wordperfect_deps="yes"
], [
	test "$enable_wordperfect" = "auto" && AC_MSG_WARN([wordperfect plugin: dependencies not satisfied - $wordperfect_pkgs])
])

fi

if test "$enable_wordperfect" = "yes" || \
   test "$wordperfect_deps" = "yes"; then

if test "$enable_wordperfect_builtin" = "yes"; then
AC_MSG_ERROR([wordperfect plugin: static linking not supported])
fi

wp_deps_pkgs="$wordperfect_pkgs"

PKG_CHECK_EXISTS([ $wordperfect_wps_pkgs ],
[
	wp_deps_pkgs="$wordperfect_wps_pkgs $wp_deps_pkgs"
	WPS_DEFINE=" -DHAVE_LIBWPS"
])

PKG_CHECK_MODULES(WORDPERFECT,[ $wp_deps_pkgs ])

test "$enable_wordperfect" = "auto" && PLUGINS="$PLUGINS wordperfect"

WORDPERFECT_CFLAGS="$WORDPERFECT_CFLAGS "'${PLUGIN_CFLAGS}'"$WPS_DEFINE"
WORDPERFECT_LIBS="$WORDPERFECT_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([WORDPERFECT_CFLAGS])
AC_SUBST([WORDPERFECT_LIBS])

