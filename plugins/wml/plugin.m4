
WML_CFLAGS=
WML_LIBS=

if test "$enable_wml" == "yes"; then

WML_CFLAGS="$WML_CFLAGS "'${PLUGIN_CFLAGS}'
WML_LIBS="$WML_LIBS "'${PLUGIN_LIBS}'

if test "$enable_wml_builtin" == "yes"; then
	WML_CFLAGS="$WML_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([WML_CFLAGS])
AC_SUBST([WML_LIBS])

