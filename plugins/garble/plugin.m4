garble_deps="no"

if test "$enable_garble" != ""; then
    if test "$TOOLKIT" != "gtk"; then
		garble_deps="no"
		AC_MSG_WARN([garble plugin: only supported on UNIX/gtk platforms])
	else 
		# stolen from the original plugin.m4 in abiword-plugins
		AC_CHECK_HEADER(readline/readline.h,[
				AC_CHECK_HEADER(readline/history.h,[
						AC_CHECK_LIB(readline,readline,[
								garble_deps="yes"
						],[     AC_CHECK_LIB(readline,rl_initialize,[
										garble_deps="yes"

								],,)
						],)
				])
		])
	fi
fi

if test "$enable_garble" == "yes" || \
   test "$garble_deps" == "yes"; then

if test "$enable_garble_builtin" == "yes"; then
AC_MSG_ERROR([garble plugin: static linking not supported])
fi

AC_MSG_CHECKING([garble plugin: for readline and friends])
if test "$garble_deps" != "yes"; then
	AC_MSG_ERROR([no])
else
	AC_MSG_RESULT([yes])
        GARBLE_LIBS="-lreadline -lhistory $GARBLE_LIBS"
fi

test "$enable_garble" == "auto" && PLUGINS="$PLUGINS garble"

GARBLE_CFLAGS="$GARBLE_CFLAGS "'${PLUGIN_CFLAGS}'
GARBLE_LIBS="$GARBLE_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([GARBLE_CFLAGS])
AC_SUBST([GARBLE_LIBS])

