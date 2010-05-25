
sdw_pkgs="$gsf_req"
sdw_deps="no"

if test "$enable_sdw" != ""; then

PKG_CHECK_EXISTS([ $sdw_pkgs ], 
[
	sdw_deps="yes"
], [
	test "$enable_sdw" = "auto" && AC_MSG_WARN([sdw plugin: dependencies not satisfied - $sdw_pkgs])
])

fi

if test "$enable_sdw" = "yes" || \
   test "$sdw_deps" = "yes"; then

PKG_CHECK_MODULES(SDW,[ $sdw_pkgs ])

test "$enable_sdw" = "auto" && PLUGINS="$PLUGINS sdw"

SDW_CFLAGS="$SDW_CFLAGS "'${PLUGIN_CFLAGS}'
SDW_LIBS="$SDW_LIBS "'${PLUGIN_LIBS}'

if test "$enable_sdw_builtin" = "yes"; then
	SDW_CFLAGS="$SDW_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([SDW_CFLAGS])
AC_SUBST([SDW_LIBS])

