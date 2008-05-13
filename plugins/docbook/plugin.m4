
docbook_pkgs="$gsf_req"

if test "$enable_docbook" == "yes"; then

AC_HEADER_TIME

PKG_CHECK_MODULES(DOCBOOK,[ $docbook_pkgs ])

DOCBOOK_CFLAGS="$DOCBOOK_CFLAGS "'${PLUGIN_CFLAGS}'
DOCBOOK_LIBS="$DOCBOOK_LIBS "'${PLUGIN_LIBS}'

if test "$enable_docbook_builtin" == "yes"; then
	DOCBOOK_CFLAGS="$DOCBOOK_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([DOCBOOK_CFLAGS])
AC_SUBST([DOCBOOK_LIBS])

