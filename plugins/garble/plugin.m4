
garble_pkgs="libgsf-1 >= 1.12 libxml-2.0 >= 2.4.0"
garble_deps="no"

if test "$enable_garble" != ""; then

PKG_CHECK_EXISTS([ $garble_pkgs ], 
[
	garble_deps="yes"
], [
	test "$enable_garble" == "auto" && AC_MSG_WARN([garble plugin: dependencies not satisfied - $garble_pkgs])
])

fi

if test "$enable_garble" == "yes" || \
   test "$garble_deps" == "yes"; then

AC_HEADER_TIME

PKG_CHECK_MODULES(GARBLE,[ $garble_pkgs ])

test "$enable_garble" == "auto" && PLUGINS="$PLUGINS garble"

GARBLE_CFLAGS="$GARBLE_CFLAGS $PNG_CFLAGS "'${PLUGIN_CFLAGS}'
GARBLE_LIBS="$GARBLE_LIBS $PNG_LIBS "'${PLUGIN_LIBS}'

if test "$enable_garble_builtin" == "yes"; then
	GARBLE_CFLAGS="$GARBLE_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([GARBLE_CFLAGS])
AC_SUBST([GARBLE_LIBS])

