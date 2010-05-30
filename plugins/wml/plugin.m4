
wml_pkgs="$gsf_req"
wml_deps="no"

if test "$enable_wml" != ""; then

PKG_CHECK_EXISTS([ $wml_pkgs ], 
[
	wml_deps="yes"
], [
	test "$enable_wml" = "auto" && AC_MSG_WARN([wml plugin: dependencies not satisfied - $wml_pkgs])
])

fi

if test "$enable_wml" = "yes" || \
   test "$wml_deps" = "yes"; then

PKG_CHECK_MODULES(WML,[ $wml_pkgs ])

test "$enable_wml" = "auto" && PLUGINS="$PLUGINS wml"

WML_CFLAGS="$WML_CFLAGS "'${PLUGIN_CFLAGS}'
WML_LIBS="$WML_LIBS "'${PLUGIN_LIBS}'

if test "$enable_wml_builtin" = "yes"; then
	WML_CFLAGS="$WML_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([WML_CFLAGS])
AC_SUBST([WML_LIBS])

