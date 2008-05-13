
hancom_pkgs="$gsf_req"

if test "$enable_hancom" == "yes"; then

PKG_CHECK_MODULES(HANCOM,[ $hancom_pkgs ])

HANCOM_CFLAGS="$HANCOM_CFLAGS "'${PLUGIN_CFLAGS}'
HANCOM_LIBS="$HANCOM_LIBS "'${PLUGIN_LIBS}'

if test "$enable_hancom_builtin" == "yes"; then
	HANCOM_CFLAGS="$HANCOM_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([HANCOM_CFLAGS])
AC_SUBST([HANCOM_LIBS])

