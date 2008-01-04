
# actually just libgnomeprint, -ui depends on it we can 
# just as well depend on what abiword proper needs anyways
command_pkgs="$libgnomeprintui_req"

COMMAND_CFLAGS=
COMMAND_LIBS=

if test "$enable_command" == "yes"; then

PKG_CHECK_MODULES(COMMAND,[ $command_pkgs ])

# stolen from the original plugin.m4 in abiword-plugins
have_readline=unknown
AC_CHECK_HEADER(readline/readline.h,[
        AC_CHECK_HEADER(readline/history.h,[
                AC_CHECK_LIB(readline,readline,[
                        have_readline=yes
                        LDFLAGS="-ltermcap $LDFLAGS"
                        COMMAND_LIBS="-ltermcap $COMMAND_LIBS"
                ],[     AC_CHECK_LIB(readline,rl_initialize,[
                                have_readline=yes
                                LDFLAGS="-lcurses $LDFLAGS"
                                COMMAND_LIBS="-lcurses $COMMAND_LIBS"
                        ],have_readline=no,-lcurses)
                ],-ltermcap)
        ],have_readline=no)
],have_readline=no)

AC_MSG_CHECKING([for readline and friends])
if [ test "$have_readline" != "yes" ]; then
	AC_MSG_ERROR([failed])
else
	AC_MSG_RESULT([ok])
        COMMAND_LIBS="-lreadline -lhistory $COMMAND_LIBS"
fi

COMMAND_CFLAGS="$COMMAND_CFLAGS "'${WP_CPPFLAGS}'
COMMAND_LIBS="$COMMAND_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([COMMAND_CFLAGS])
AC_SUBST([COMMAND_LIBS])

