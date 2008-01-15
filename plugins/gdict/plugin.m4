
GDICT_CFLAGS="-DUSE_FORK_AND_EXEC_METHOD=1"
GDICT_LIBS=

if test "$enable_gdict" == "yes"; then

AC_MSG_CHECKING([for unix platform])
if test "$PLATFORM" == "unix"; then
  AC_MSG_RESULT([ok])
else
  AC_MSG_ERROR([gdict plugin: only supported on UNIX platform])
fi

AC_TYPE_PID_T

GDICT_CFLAGS="$GDICT_CFLAGS "'${PLUGIN_CFLAGS}'
GDICT_LIBS='${PLUGIN_LIBS}'

fi

AC_SUBST([GDICT_CFLAGS])
AC_SUBST([GDICT_LIBS])

