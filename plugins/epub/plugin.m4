
epub_pkgs="libgsf-1 >= 1.14.4"
epub_deps="no"

if test "$enable_epub" != ""; then

PKG_CHECK_EXISTS([ $epub_pkgs ], 
[
	epub_deps="yes"
	], [
	test "$enable_epub" = "auto" && AC_MSG_WARN([epub plugin: dependencies not satisfied - $epub_pkgs])
])

fi

if test "$enable_epub" = "yes" || \
   test "$epub_deps" = "yes"; then

PKG_CHECK_MODULES(EPUB,[ $epub_pkgs ])

test "$enable_epub" = "auto" && PLUGINS="$PLUGINS epub"

EPUB_CFLAGS="$EPUB_CFLAGS "'${PLUGIN_CFLAGS}'
EPUB_LIBS="$EPUB_LIBS "'${PLUGIN_LIBS}'

if test "$enable_epub_builtin" = "yes"; then
	EPUB_CFLAGS="$EPUB_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([EPUB_CFLAGS])
AC_SUBST([EPUB_LIBS])

