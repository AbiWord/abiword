if test "$TOOLKIT" == "gtk" && test "$enable_goffice" != ""; then

test "$enable_goffice" = "auto" && PLUGINS="$PLUGINS goffice"

GOFFICE_CFLAGS='${PLUGIN_CFLAGS}'
GOFFICE_LIBS='${PLUGIN_LIBS}'

fi

AC_SUBST([GOFFICE_CFLAGS])
AC_SUBST([GOFFICE_LIBS])

