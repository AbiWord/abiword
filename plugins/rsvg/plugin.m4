
rsvg_pkgs="librsvg-2.0 >= 2.0 glib-2.0"
rsvg_deps="no"

if test "$TOOLKIT" = "gtk"; then
 
 if test "$enable_rsvg" = "auto"; then
   AC_MSG_WARN([rsvg plugin: not needed with gtk])
 fi
 enable_rsvg=""
fi

if test "$enable_rsvg" != ""; then

PKG_CHECK_EXISTS([ $rsvg_pkgs ], 
[
	rsvg_deps="yes"
], [
	test "$enable_rsvg" = "auto" && AC_MSG_WARN([rsvg plugin: dependencies not satisfied - $rsvg_pkgs])
])

fi

if test "$enable_rsvg" = "yes" || \
   test "$rsvg_deps" = "yes"; then

if test "$enable_rsvg_builtin" = "yes"; then
AC_MSG_ERROR([rsvg plugin: static linking not supported])
fi

PKG_CHECK_MODULES(RSVG,[ $rsvg_pkgs ])

test "$enable_rsvg" = "auto" && PLUGINS="$PLUGINS rsvg"

RSVG_CFLAGS="$RSVG_CFLAGS "'${PLUGIN_CFLAGS}'
RSVG_LIBS="$RSVG_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([RSVG_CFLAGS])
AC_SUBST([RSVG_LIBS])

