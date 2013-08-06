lasem
dnl Only 0.4.10, or later will work
for ver in 0.6 0.4 ; do
  if test "x$lasem" = x; then
    if pkg-config --exists lasem-$ver; then
      lasem=lasem-$ver
    fi
  fi
done
if test "x$lasem" = x; then
  # Not important.  Things will fail below.
  lasem=lasem-0.4
fi
mathview_pkgs='$lasem-0.4 >= 0.4.1'
AC_CHECK_FUNCS(lsm)
PKG_CHECK_MODULES(MATHVIEW,[ $mathview_pkgs ])

test "$enable_mathview" = "auto" && PLUGINS="$PLUGINS mathview"

MATHVIEW_CFLAGS="$MATHVIEW_CFLAGS"'${PLUGIN_CFLAGS}'
MATHVIEW_LIBS="$MATHVIEW_LIBS "'${PLUGIN_LIBS}'

AC_SUBST([MATHVIEW_CFLAGS])
AC_SUBST([MATHVIEW_LIBS])

dnl check for lsm_itex_to_mathml()
saved_CFLAGS=$CFLAGS
saved_LIBS=$LIBS
CFLAGS=$MATHVIEW_CFLAGS
LIBS=$MATHVIEW_LIBS
AC_CHECK_FUNCS(lsm_itex_to_mathml)
CFLAGS=$saved_CFLAGS
LIBS=$saved_CLIBS




mathview_deps="no"


# test hashmap availablity
HASHMAP_CFLAGS=""
AC_LANG(C++)
AC_CHECK_HEADER(hash_map,
[
	HASHMAP_CFLAGS="-DHAVE_HASH_MAP"
], [
	AC_CHECK_HEADER(ext/hash_map,
	[
		HASHMAP_CFLAGS="-DHAVE_EXT_HASH_MAP"
	], [
		AC_MSG_WARN([mathview plugin: dependencies not satisfied - missing 'hash_map' or 'ext/hash_map' header])
	])
])
AC_LANG(C)

if test "$HASHMAP_CFLAGS" != ""; then

if test "$enable_mathview" != ""; then

PKG_CHECK_EXISTS([ $mathview_pkgs ], 
[
	mathview_deps="yes"
], [
	test "$enable_mathview" = "auto" && AC_MSG_WARN([mathview plugin: dependencies not satisfied - $mathview_pkgs])
])

fi

if test "$enable_mathview" = "yes" || \
   test "$mathview_deps" = "yes"; then

if test "$enable_mathview_builtin" = "yes"; then
AC_MSG_ERROR([mathview plugin: static linking not supported])
fi

PKG_CHECK_MODULES(MATHVIEW,[ $mathview_pkgs ])

test "$enable_mathview" = "auto" && PLUGINS="$PLUGINS mathview"

MATHVIEW_CFLAGS="$MATHVIEW_CFLAGS $HASHMAP_CFLAGS "'${PLUGIN_CFLAGS}'
MATHVIEW_LIBS="$MATHVIEW_LIBS "'${PLUGIN_LIBS}'

fi

fi

# need to unconditionally test, for `make distcheck'
AM_PROG_LEX
AC_PROG_YACC

AC_SUBST([MATHVIEW_CFLAGS])
AC_SUBST([MATHVIEW_LIBS])

