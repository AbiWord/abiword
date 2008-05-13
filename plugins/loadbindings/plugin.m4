
loadbindings_pkgs="$gsf_req"

if test "$enable_loadbindings" == "yes"; then

PKG_CHECK_MODULES(LOADBINDINGS,[ $loadbindings_pkgs ])

LOADBINDINGS_CFLAGS="$LOADBINDINGS_CFLAGS "'${PLUGIN_CFLAGS}'
LOADBINDINGS_LIBS="$LOADBINDINGS_LIBS "'${PLUGIN_LIBS}'

if test "$enable_loadbindings_builtin" == "yes"; then
	LOADBINDINGS_CFLAGS="$LOADBINDINGS_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([LOADBINDINGS_CFLAGS])
AC_SUBST([LOADBINDINGS_LIBS])

