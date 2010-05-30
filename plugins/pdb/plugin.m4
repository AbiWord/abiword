
pdb_pkgs="$gsf_req"
pdb_deps="no"

if test "$enable_pdb" != ""; then

PKG_CHECK_EXISTS([ $pdb_pkgs ], 
[
	pdb_deps="yes"
], [
	test "$enable_pdb" = "auto" && AC_MSG_WARN([pdb plugin: dependencies not satisfied - $pdb_pkgs])
])

fi

if test "$enable_pdb" = "yes" || \
   test "$pdb_deps" = "yes"; then

PKG_CHECK_MODULES(PDB,[ $pdb_pkgs ])

test "$enable_pdb" = "auto" && PLUGINS="$PLUGINS pdb"

PDB_CFLAGS="$PDB_CFLAGS "'${PLUGIN_CFLAGS}'
PDB_LIBS="$PDB_LIBS "'${PLUGIN_LIBS}'

if test "$enable_pdb_builtin" = "yes"; then
	PDB_CFLAGS="$PDB_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([PDB_CFLAGS])
AC_SUBST([PDB_LIBS])

