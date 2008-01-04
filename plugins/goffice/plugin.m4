
goffice_pkgs="$goffice_req"

GOFFICE_CFLAGS=
GOFFICE_LIBS=

if test "$enable_goffice" == "yes"; then

PKG_CHECK_MODULES(GOFFICE,[ $goffice_pkgs ])

AC_CHECK_HEADER(hash_map,[GOFFICE_CFLAGS="-DHAVE_HASH_MAP $GOFFICE_CFLAGS"],[
AC_CHECK_HEADER(ext/hash_map,[GOFFICE_CFLAGS="-DHAVE_EXT_HASH_MAP $GOFFICE_CFLAGS"],
[        
	AC_MSG_WARN([goffice: error hash_map header not found])])
])

GOFFICE_CFLAGS="$GOFFICE_CFLAGS "'${WP_CPPFLAGS}'
GOFFICE_LIBS="$GOFFICE_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([GOFFICE_CFLAGS])
AC_SUBST([GOFFICE_LIBS])

