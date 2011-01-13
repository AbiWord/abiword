
grammar_pkgs="accidence >= 0.2.0"
grammar_deps="no"

if test "$enable_grammar" != ""; then

PKG_CHECK_EXISTS([ $grammar_pkgs ], 
[
	grammar_deps="yes"
], [
	grammar_deps="no"
])

if test "$grammar_deps" = "yes"; then
	AC_CHECK_PROG(HAVE_ICU_CONFIG, icu-config, yes, no)

	if test "$HAVE_ICU_CONFIG" = "yes"; then
		ICU_CFLAGS=`icu-config --cflags`
		ICU_LIBS=`icu-config --ldflags`
	else
		grammar_deps="no"
	fi
fi

fi

if test "$enable_grammar" = "yes" || \
   test "$grammar_deps" = "yes"; then

if test "$enable_grammar_builtin" = "yes"; then
AC_MSG_ERROR([grammar plugin: static linking not supported])
fi

PKG_CHECK_MODULES(GRAMMAR,[ $grammar_pkgs ])

test "$enable_grammar" = "auto" && PLUGINS="$PLUGINS grammar"

GRAMMAR_CFLAGS="$GRAMMAR_CFLAGS $ICU_CFLAGS "'${PLUGIN_CFLAGS}'
GRAMMAR_LIBS="$GRAMMAR_LIBS $ICU_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([GRAMMAR_CFLAGS])
AC_SUBST([GRAMMAR_LIBS])

