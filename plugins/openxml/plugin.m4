
openxml_pkgs="libgsf-1 >= 1.14.4"
openxml_deps="no"

if test "$enable_openxml" != ""; then

PKG_CHECK_EXISTS([ $openxml_pkgs ], 
[
	AC_LANG_PUSH(C++)
	AC_CHECK_HEADER([boost/shared_ptr.hpp], 
	[
	  openxml_deps="yes"
	], [
		if test "$enable_openxml" = "auto"; then
		  AC_MSG_WARN([openxml plugin: `boost/shared_ptr.hpp' not found, install boost or specify CPPFLAGS to include custom locations])
		else
		  AC_MSG_ERROR([openxml plugin: `boost/shared_ptr.hpp' not found, install boost or specify CPPFLAGS to include custom locations])
		fi
	])
	AC_LANG_POP
], [
	test "$enable_openxml" = "auto" && AC_MSG_WARN([openxml plugin: dependencies not satisfied - $openxml_pkgs])
])

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

