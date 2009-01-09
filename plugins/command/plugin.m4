
# actually just libgnomeprint, -ui depends on it we can 
# just as well depend on what abiword proper needs anyways
command_pkgs="$libgnomeprintui_req"
command_deps="no"

if test "$enable_command" != ""; then

PKG_CHECK_EXISTS([ $command_pkgs ], 
[
	# stolen from the original plugin.m4 in abiword-plugins
	AC_CHECK_HEADER(readline/readline.h,[
	        AC_CHECK_HEADER(readline/history.h,[
	                AC_CHECK_LIB(readline,readline,[
	                        command_deps="yes"
	                        COMMAND_LIBS="-ltermcap $COMMAND_LIBS"
	                ],[     AC_CHECK_LIB(readline,rl_initialize,[
	                                command_deps="yes"
	                                COMMAND_LIBS="-lcurses $COMMAND_LIBS"
	                        ],,-lcurses)
	                ],-ltermcap)
	        ])
	])

], [
	test "$enable_command" == "auto" && AC_MSG_WARN([command plugin: dependencies not satisfied - $command_pkgs])
])

fi

if test "$enable_command" == "yes" || \
   test "$command_deps" == "yes"; then

if test "$enable_command_builtin" == "yes"; then
AC_MSG_ERROR([command plugin: static linking not supported])
fi

PKG_CHECK_MODULES(COMMAND,[ $command_pkgs ])

AC_MSG_CHECKING([command plugin: for readline and friends])
if test "$command_deps" != "yes"; then
	AC_MSG_ERROR([no])
else
	AC_MSG_RESULT([yes])
        COMMAND_LIBS="-lreadline -lhistory $COMMAND_LIBS"
fi

test "$enable_command" == "auto" && PLUGINS="$PLUGINS command"

COMMAND_CFLAGS="$COMMAND_CFLAGS "'${PLUGIN_CFLAGS}'
COMMAND_LIBS="$COMMAND_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([COMMAND_CFLAGS])
AC_SUBST([COMMAND_LIBS])

