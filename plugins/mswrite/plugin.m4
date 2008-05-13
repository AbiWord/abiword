
mswrite_pkgs="$gsf_req"

if test "$enable_mswrite" == "yes"; then

PKG_CHECK_MODULES(MSWRITE,[ $mswrite_pkgs ])

MSWRITE_CFLAGS="$MSWRITE_CFLAGS "'${PLUGIN_CFLAGS}'
MSWRITE_LIBS="$MSWRITE_LIBS "'${PLUGIN_LIBS}'

if test "$enable_mswrite_builtin" == "yes"; then
	MSWRITE_CFLAGS="$MSWRITE_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([MSWRITE_CFLAGS])
AC_SUBST([MSWRITE_LIBS])

