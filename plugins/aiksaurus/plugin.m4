
aiksaurus_pkgs="aiksaurus-1.0"
aiksaurus_gtk_pkgs="gaiksaurus-1.0"
aiksaurus_deps="no"

if test "$enable_aiksaurus" != ""; then

PKG_CHECK_EXISTS([ $aiksaurus_pkgs ], 
[
	if test "$TOOLKIT" = "gtk"; then
	PKG_CHECK_EXISTS([ $aiksaurus_gtk_pkgs ], 
	[
		aiksaurus_deps="yes"
	], [
		test "$enable_aiksaurus" = "auto" && AC_MSG_WARN([aiksaurus plugin: dependencies not satisfied - $aiksaurus_gtk_pkgs])
	])
	else
	  aiksaurus_deps="yes"
	fi
], [
	test "$enable_aiksaurus" = "auto" && AC_MSG_WARN([aiksaurus plugin: dependencies not satisfied - $aiksaurus_pkgs])
])

fi

if test "$enable_aiksaurus" = "yes" || \
   test "$aiksaurus_deps" = "yes"; then

if test "$enable_aiksaurus_builtin" = "yes"; then
AC_MSG_ERROR([aiksaurus plugin: static linking not supported])
fi

PKG_CHECK_MODULES(AIKSAURUS,[ $aiksaurus_pkgs ])

if test "$TOOLKIT" = "gtk"; then
	PKG_CHECK_MODULES(AIKSAURUS_GTK,[ $aiksaurus_gtk_pkgs ])
	AIKSAURUS_CFLAGS="$AIKSAURUS_CFLAGS $AIKSAURUS_GTK_CFLAGS"
	AIKSAURUS_LIBS="$AIKSAURUS_LIBS $AIKSAURUS_GTK_LIBS"
fi

test "$enable_aiksaurus" = "auto" && PLUGINS="$PLUGINS aiksaurus"

AIKSAURUS_CFLAGS="$AIKSAURUS_CFLAGS "'${PLUGIN_CFLAGS}'
AIKSAURUS_LIBS="$AIKSAURUS_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([AIKSAURUS_CFLAGS])
AC_SUBST([AIKSAURUS_LIBS])

