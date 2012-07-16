
openxml_pkgs="libgsf-1 >= 1.14.4"
openxml_deps="no"

if test "$enable_openxml" != ""; then

PKG_CHECK_EXISTS([ $openxml_pkgs ], 
[
	openxml_deps="yes"
	], [
	test "$enable_openxml" = "auto" && AC_MSG_WARN([openxml plugin: dependencies not satisfied - $openxml_pkgs])
])

AC_SUBST(ABIWORD_OMMLXSLTDIR, "${ABIWORD_DATADIR}/omml_xslt")

fi

if test "$enable_openxml" = "yes" || \
   test "$openxml_deps" = "yes"; then

PKG_CHECK_MODULES(OPENXML,[ $openxml_pkgs ])

test "$enable_openxml" = "auto" && PLUGINS="$PLUGINS openxml"

OPENXML_CFLAGS="$OPENXML_CFLAGS "'${PLUGIN_CFLAGS}'
OPENXML_LIBS="$OPENXML_LIBS "'${PLUGIN_LIBS}'

if test "$enable_openxml_builtin" = "yes"; then
	OPENXML_CFLAGS="$OPENXML_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([OPENXML_CFLAGS])
AC_SUBST([OPENXML_LIBS])

