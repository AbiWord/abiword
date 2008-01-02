
gda_pkgs='libgda >= 1.2.0 libgnomedb >= 1.2.0'

GDA_CFLAGS=
GDA_LIBS=

if test "$enable_gda" == "yes"; then

PKG_CHECK_MODULES(GDA,[ $gda_pkgs ])

GDA_CFLAGS="$GDA_CFLAGS"'${WP_CPPFLAGS}'
GDA_LIBS="$GDA_LIBS"'${PLUGIN_LIBS}'

fi

AC_SUBST([GDA_CFLAGS])
AC_SUBST([GDA_LIBS])

