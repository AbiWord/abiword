
pdb_pkgs="$gsf_req"

PDB_CFLAGS=
PDB_LIBS=

if test "$enable_pdb" == "yes"; then

PKG_CHECK_MODULES(PDB,[ $pdb_pkgs ])

PDB_CFLAGS="$PDB_CFLAGS"'${WP_CPPFLAGS}'
PDB_LIBS="$PDB_LIBS"'${PLUGIN_LIBS}'

fi

AC_SUBST([PDB_CFLAGS])
AC_SUBST([PDB_LIBS])

