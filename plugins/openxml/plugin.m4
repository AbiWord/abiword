
openxml_pkgs="libgsf-1 >= 1.14.4"

OPENXML_CFLAGS=
OPENXML_LIBS=

if test "$enable_openxml" == "yes"; then

PKG_CHECK_MODULES(OPENXML,[ $openxml_pkgs ])

OPENXML_CFLAGS="$OPENXML_CFLAGS "'${PLUGIN_CFLAGS}'
OPENXML_LIBS="$OPENXML_LIBS "'${PLUGIN_LIBS}'

AC_LANG_PUSH(C++)
AC_CHECK_HEADER([boost/shared_ptr.hpp], [], 
[
	AC_MSG_ERROR([`boost/shared_ptr.hpp' not found, install boost or specify CPPFLAGS to include custom locations])
])
AC_LANG_POP

fi

AC_SUBST([OPENXML_CFLAGS])
AC_SUBST([OPENXML_LIBS])

