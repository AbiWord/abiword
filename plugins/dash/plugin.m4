
DASH_CFLAGS=
DASH_LIBS=
dash_deps="no"

if test "$enable_dash" != ""; then

AC_MSG_CHECKING([for unix platform])
if test "$PLATFORM" == "unix"; then
  AC_MSG_RESULT([yes])
  dash_deps="yes"
else
  AC_MSG_RESULT([no])
  if test "$enable_dash" == "auto"; then
    AC_MSG_WARN([dash plugin: only supported on UNIX platforms])
  else
    AC_MSG_ERROR([dash plugin: only supported on UNIX platforms])
  fi
fi

fi

if test "$enable_dash" == "yes" || \
   test "$dash_deps" == "yes"; then

test "$enable_dash" == "auto" && PLUGINS="$PLUGINS dash"

DASH_CFLAGS="$DASH_CFLAGS "'${PLUGIN_CFLAGS}'
DASH_LIBS="$DASH_LIBS "'${PLUGIN_LIBS}'

if test "$enable_dash_builtin" == "yes"; then
	DASH_CFLAGS="$DASH_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([DASH_CFLAGS])
AC_SUBST([DASH_LIBS])

