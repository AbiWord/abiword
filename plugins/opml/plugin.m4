
OPML_CFLAGS=
OPML_LIBS=

if test "$enable_opml" != ""; then

test "$enable_opml" = "auto" && PLUGINS="$PLUGINS opml"

OPML_CFLAGS="$OPML_CFLAGS "'${PLUGIN_CFLAGS}'
OPML_LIBS="$OPML_LIBS "'${PLUGIN_LIBS}'

if test "$enable_opml_builtin" = "yes"; then
	OPML_CFLAGS="$OPML_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([OPML_CFLAGS])
AC_SUBST([OPML_LIBS])

