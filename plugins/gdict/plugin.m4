
GDICT_CFLAGS="-DUSE_FORK_AND_EXEC_METHOD=1"
GDICT_LIBS=

if test "$enable_gdict" == "yes"; then

AC_TYPE_PID_T

GDICT_CFLAGS="$GDICT_CFLAGS "'${WP_CPPFLAGS}'

fi

AC_SUBST([GDICT_CFLAGS])
AC_SUBST([GDICT_LIBS])

