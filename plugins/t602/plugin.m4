
t602_pkgs="$gsf_req"
t602_deps="no"

if test "$enable_t602" != ""; then

PKG_CHECK_EXISTS([ $t602_pkgs ], 
[
	t602_deps="yes"
], [
	test "$enable_t602" = "auto" && AC_MSG_WARN([t602 plugin: dependencies not satisfied - $t602_pkgs])
])

fi

if test "$enable_t602" = "yes" || \
   test "$t602_deps" = "yes"; then

PKG_CHECK_MODULES(T602,[ $t602_pkgs ])

test "$enable_t602" = "auto" && PLUGINS="$PLUGINS t602"

T602_CFLAGS="$T602_CFLAGS "'${PLUGIN_CFLAGS}'
T602_LIBS="$T602_LIBS "'${PLUGIN_LIBS}'

if test "$enable_t602_builtin" = "yes"; then
	T602_CFLAGS="$T602_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([T602_CFLAGS])
AC_SUBST([T602_LIBS])

