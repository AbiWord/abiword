
wpg_pkgs="$gsf_req libwpg-0.3 librevenge-0.0"
wpg_deps="no"

if test "$enable_wpg" != ""; then

PKG_CHECK_EXISTS([ $wpg_pkgs ], 
[
	wpg_deps="yes"
], [
	test "$enable_wpg" = "auto" && AC_MSG_WARN([wpg plugin: dependencies not satisfied - $wpg_pkgs])
])

fi

if test "$enable_wpg" = "yes" || \
   test "$wpg_deps" = "yes"; then

if test "$enable_wpg_builtin" = "yes"; then
AC_MSG_ERROR([wpg plugin: static linking not supported])
fi

PKG_CHECK_MODULES(WPG, [ $wpg_pkgs ])

test "$enable_wpg" = "auto" && PLUGINS="$PLUGINS wpg"

WPG_CFLAGS="$WPG_CFLAGS "'${PLUGIN_CFLAGS}'
WPG_LIBS="$WPG_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([WPG_CFLAGS])
AC_SUBST([WPG_LIBS])

