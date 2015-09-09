
grammar_pkgs='link-grammar >= 4.2.1'
grammar_deps="no"

dnl make sure we enable grammar only if spell is enabled. At least in auto mode.
if test "$enable_grammar" != "" && test  "$abi_cv_spell" = "yes"; then

PKG_CHECK_EXISTS([ $grammar_pkgs ], 
[
	grammar_deps="yes"
], [
	test "$enable_grammar" = "auto" && AC_MSG_WARN([grammar plugin: dependencies not satisfied - $grammar_pkgs])
])

fi

if test "$enable_grammar" = "yes" || \
   test "$grammar_deps" = "yes"; then

if test "$enable_grammar_builtin" = "yes"; then
AC_MSG_ERROR([grammar plugin: static linking not supported])
fi

PKG_CHECK_MODULES(GRAMMAR,[ $grammar_pkgs ])
PKG_CHECK_EXISTS([ link-grammar >= 5.1.0 ], 
[
	AC_DEFINE([HAVE_LINK_GRAMMAR_51],[1],["have link-grammar 5.1.0 or later"])
])

test "$enable_grammar" = "auto" && PLUGINS="$PLUGINS grammar"

GRAMMAR_CFLAGS="$GRAMMAR_CFLAGS "'${PLUGIN_CFLAGS}'
GRAMMAR_LIBS="$GRAMMAR_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([GRAMMAR_CFLAGS])
AC_SUBST([GRAMMAR_LIBS])

