
xslfo_pkgs="$gsf_req"
xslfo_deps="no"

if test "$enable_xslfo" != ""; then

PKG_CHECK_EXISTS([ $xslfo_pkgs ], 
[
	xslfo_deps="yes"
], [
	test "$enable_xslfo" = "auto" && AC_MSG_WARN([xslfo plugin: dependencies not satisfied - $xslfo_pkgs])
])

fi

if test "$enable_xslfo" = "yes" || \
   test "$xslfo_deps" = "yes"; then

PKG_CHECK_MODULES(XSLFO,[ $xslfo_pkgs ])

test "$enable_xslfo" = "auto" && PLUGINS="$PLUGINS xslfo"

XSLFO_CFLAGS="$XSLFO_CFLAGS "'${PLUGIN_CFLAGS}'
XSLFO_LIBS="$XSLFO_LIBS "'${PLUGIN_LIBS}'

if test "$enable_xslfo_builtin" = "yes"; then
	XSLFO_CFLAGS="$XSLFO_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([XSLFO_CFLAGS])
AC_SUBST([XSLFO_LIBS])

