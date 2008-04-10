
SDW_CFLAGS=
SDW_LIBS=

if test "$enable_sdw" == "yes"; then

SDW_CFLAGS="$SDW_CFLAGS "'${PLUGIN_CFLAGS}'
SDW_LIBS="$SDW_LIBS "'${PLUGIN_LIBS}'

if test "$enable_sdw_builtin" == "yes"; then
	SDW_CFLAGS="$SDW_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([SDW_CFLAGS])
AC_SUBST([SDW_LIBS])

