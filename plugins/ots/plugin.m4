
ots_pkgs="libots-1 >= 0.5.0"
ots_deps="no"

if test "$enable_ots" != ""; then

PKG_CHECK_EXISTS([ $ots_pkgs ], 
[
	ots_deps="yes"
], [
	test "$enable_ots" = "auto" && AC_MSG_WARN([ots plugin: dependencies not satisfied - $ots_pkgs])
])

fi

if test "$enable_ots" = "yes" || \
   test "$ots_deps" = "yes"; then

test "$enable_ots" = "auto" && PLUGINS="$PLUGINS ots"

if test "$enable_ots_builtin" = "yes"; then
AC_MSG_ERROR([ots plugin: static linking not supported])
fi

PKG_CHECK_MODULES(OTS,[ $ots_pkgs ])

OTS_CFLAGS="$OTS_CFLAGS "'${PLUGIN_CFLAGS}'
OTS_LIBS="$OTS_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([OTS_CFLAGS])
AC_SUBST([OTS_LIBS])

