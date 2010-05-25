
openwriter_pkgs="$gsf_req"
openwriter_deps="no"

if test "$enable_openwriter" != ""; then

PKG_CHECK_EXISTS([ $openwriter_pkgs ], 
[
	openwriter_deps="yes"
], [
	test "$enable_openwriter" = "auto" && AC_MSG_WARN([openwriter plugin: dependencies not satisfied - $openwriter_pkgs])
])

fi

if test "$enable_openwriter" = "yes" || \
   test "$openwriter_deps" = "yes"; then

PKG_CHECK_MODULES(OPENWRITER,[ $openwriter_pkgs ])

test "$enable_openwriter" = "auto" && PLUGINS="$PLUGINS openwriter"

OPENWRITER_CFLAGS="$OPENWRITER_CFLAGS "'${PLUGIN_CFLAGS}'
OPENWRITER_LIBS="$OPENWRITER_LIBS "'${PLUGIN_LIBS}'

if test "$enable_openwriter_builtin" = "yes"; then
	OPENWRITER_CFLAGS="$OPENWRITER_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([OPENWRITER_CFLAGS])
AC_SUBST([OPENWRITER_LIBS])

