
grammar_pkgs='link-grammar >= 4.2.1'

GRAMMAR_CFLAGS=
GRAMMAR_LIBS=

if test "$enable_grammar" == "yes"; then

if test "$enable_grammar_builtin" == "yes"; then
AC_MSG_ERROR([static linking is not supported for the `grammar' plugin])
fi

PKG_CHECK_MODULES(GRAMMAR,[ $grammar_pkgs ])

GRAMMAR_CFLAGS="$GRAMMAR_CFLAGS "'${PLUGIN_CFLAGS}'
GRAMMAR_LIBS="$GRAMMAR_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([GRAMMAR_CFLAGS])
AC_SUBST([GRAMMAR_LIBS])

