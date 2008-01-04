
grammar_pkgs='link-grammar >= 4.2.1'

GRAMMAR_CFLAGS=
GRAMMAR_LIBS=

if test "$enable_grammar" == "yes"; then

PKG_CHECK_MODULES(GRAMMAR,[ $grammar_pkgs ])

GRAMMAR_CFLAGS="$GRAMMAR_CFLAGS "'${WP_CPPFLAGS}'
GRAMMAR_LIBS="$GRAMMAR_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([GRAMMAR_CFLAGS])
AC_SUBST([GRAMMAR_LIBS])

