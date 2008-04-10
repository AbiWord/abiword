
PDB_CFLAGS=
PDB_LIBS=

if test "$enable_pdb" == "yes"; then

PDB_CFLAGS="$PDB_CFLAGS "'${PLUGIN_CFLAGS}'
PDB_LIBS="$PDB_LIBS "'${PLUGIN_LIBS}'

if test "$enable_pdb_builtin" == "yes"; then
	PDB_CFLAGS="$PDB_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([PDB_CFLAGS])
AC_SUBST([PDB_LIBS])

