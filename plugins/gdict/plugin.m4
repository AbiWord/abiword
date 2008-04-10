
GDICT_CFLAGS=
GDICT_LIBS=

if test "$enable_gdict" == "yes"; then

AC_MSG_CHECKING([for unix platform])
if test "$PLATFORM" == "unix"; then
  AC_MSG_RESULT([ok])
else
  AC_MSG_ERROR([gdict plugin: only supported on UNIX platform])
fi

AC_TYPE_PID_T

GDICT_CFLAGS="$GDICT_CFLAGS "'${PLUGIN_CFLAGS} -DUSE_FORK_AND_EXEC_METHOD=1'
GDICT_LIBS='${PLUGIN_LIBS}'

if test "$enable_gdict_builtin" == "yes"; then
	GDICT_CFLAGS="$GDICT_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([GDICT_CFLAGS])
AC_SUBST([GDICT_LIBS])

