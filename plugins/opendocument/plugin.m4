
opendocument_pkgs="$gsf_req"
opendocument_deps="no"

if test "$enable_opendocument" != ""; then

PKG_CHECK_EXISTS([ $opendocument_pkgs ], 
[
	opendocument_deps="yes"
], [
	test "$enable_opendocument" == "auto" && AC_MSG_WARN([opendocument plugin: dependencies not satisfied - $opendocument_pkgs])
])

fi

if test "$enable_opendocument" == "yes" || \
   test "$opendocument_deps" == "yes"; then

PKG_CHECK_MODULES(OPENDOCUMENT,[ $opendocument_pkgs ])

test "$enable_opendocument" == "auto" && PLUGINS="$PLUGINS opendocument"

OPENDOCUMENT_CFLAGS="$OPENDOCUMENT_CFLAGS "'${PLUGIN_CFLAGS}'
OPENDOCUMENT_LIBS="$OPENDOCUMENT_LIBS "'${PLUGIN_LIBS} -lz'

if test "$enable_opendocument_builtin" == "yes"; then
	OPENDOCUMENT_CFLAGS="$OPENDOCUMENT_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([OPENDOCUMENT_CFLAGS])
AC_SUBST([OPENDOCUMENT_LIBS])

