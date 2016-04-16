lasem=

PKG_CHECK_EXISTS(libgoffice-0.10 >= 0.10.12, , [enable_mathview=no])

if test "$enable_mathview" != "no"; then
  dnl Only 0.4.1, or later will work
  for ver in 0.6 0.4 ; do
    if test "x$lasem" = x; then
      if pkg-config --exists lasem-$ver; then
        lasem="lasem-$ver"
      fi
    fi
  done
  if test "x$lasem" = x; then
    test "$enable_mathview" = "auto" && AC_MSG_WARN([mathview plugin: dependencies not satisfied - $mathview_pkgs])
  fi
fi
mathview_pkgs="$lasem >= 0.4.1"

if test "$enable_mathview" = "yes" || \
   test "x$lasem" != x; then

  if test "$enable_mathview_builtin" = "yes"; then
    AC_MSG_ERROR([mathview plugin: static linking not supported])
  fi

  PKG_CHECK_MODULES(MATHVIEW,[ $mathview_pkgs ])

  test "$enable_mathview" = "auto" && PLUGINS="$PLUGINS mathview"

  dnl check for lsm_itex_to_mathml()
  saved_CFLAGS=$CFLAGS
  saved_LIBS=$LIBS
  CFLAGS=$MATHVIEW_CFLAGS
  LIBS=$MATHVIEW_LIBS
  AC_CHECK_FUNCS(lsm_itex_to_mathml)
  CFLAGS=$saved_CFLAGS
  LIBS=$saved_CLIBS

  MATHVIEW_CFLAGS="$MATHVIEW_CFLAGS "'${PLUGIN_CFLAGS}'
  MATHVIEW_LIBS="$MATHVIEW_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([MATHVIEW_CFLAGS])
AC_SUBST([MATHVIEW_LIBS])


