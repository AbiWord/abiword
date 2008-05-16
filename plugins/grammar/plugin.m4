
grammar_pkgs='link-grammar >= 4.2.1'
grammar_deps="no"

if test "$enable_grammar" != ""; then

PKG_CHECK_EXISTS([ $grammar_pkgs ], 
[
	grammar_deps="yes"
], [
	test "$enable_grammar" == "auto" && AC_MSG_WARN([grammar plugin: dependencies not satisfied - $grammar_pkgs])
])

fi

if test "$enable_grammar" == "yes" || \
   test "$grammar_deps" == "yes"; then

if test "$enable_grammar_builtin" == "yes"; then
AC_MSG_ERROR([grammar plugin: static linking not supported])
fi

PKG_CHECK_MODULES(GRAMMAR,[ $grammar_pkgs ])

PLUGINS="$PLUGINS grammar"

GRAMMAR_CFLAGS="$GRAMMAR_CFLAGS "'${PLUGIN_CFLAGS}'
GRAMMAR_LIBS="$GRAMMAR_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([GRAMMAR_CFLAGS])
AC_SUBST([GRAMMAR_LIBS])

