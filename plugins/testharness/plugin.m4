testharness_deps="yes"

if test "$enable_testharness" = "yes" || \
   test "$testharness_deps" = "yes"; then

if test "$enable_testharness_builtin" = "yes"; then
AC_MSG_ERROR([testharness plugin: static linking not supported])
fi

test "$enable_testharness" = "auto" && PLUGINS="$PLUGINS testharness"

TESTHARNESS_CFLAGS="$TESTHARNESS_CFLAGS "'${PLUGIN_CFLAGS}'
TESTHARNESS_LIBS="$TESTHARNESS_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([TESTHARNESS_CFLAGS])
AC_SUBST([TESTHARNESS_LIBS])

