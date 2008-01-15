
gda_pkgs='libgda >= 1.2.0 libgnomedb >= 1.2.0'

GDA_CFLAGS=
GDA_LIBS=

if test "$enable_gda" == "yes"; then

AC_MSG_CHECKING([for gtk toolkit])
if test "$TOOLKIT" == "gtk"; then
  AC_MSG_RESULT([ok])
else
  AC_MSG_ERROR([gda plugin: only supported with gtk])
fi

PKG_CHECK_MODULES(GDA,[ $gda_pkgs ])

GDA_CFLAGS="$GDA_CFLAGS "'${PLUGIN_CFLAGS}'
GDA_LIBS="$GDA_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([GDA_CFLAGS])
AC_SUBST([GDA_LIBS])

