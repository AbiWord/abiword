
AC_ARG_WITH([psiconv-config],
	[AS_HELP_STRING([--with-psiconv-config=DIR], [use psiconv-config in DIR])],
[
	AC_PATH_PROG(psiconvconfig, psiconv-config, , "$withval")
], [
	AC_PATH_PROG(psiconvconfig, psiconv-config)
])

# The required psiconv version, as reported by psiconv-config
psiconv_major_req=0
psiconv_minor_req=9
psiconv_micro_req=4
psion_deps="no"

if test "$enable_psion" != ""; then

	if test "$psiconvconfig" = ""; then
		if test "$enable_psion" = "yes"; then
		  AC_MSG_ERROR([psiconv plugin: program psiconv-config not found in path])
		else
		  AC_MSG_WARN([psiconv plugin: program psiconv-config not found in path])
		fi
	else
		IFS_old="$IFS"
		IFS='.'
		set -- `$psiconvconfig --version`
		psiconv_major_found="${1}"
		psiconv_minor_found="${2}"
		psiconv_micro_found="${3}"
		IFS="$IFS_old"
		if test "$psiconv_major_found" -gt "$psiconv_major_req"; then
			psion_deps="yes"
		elif test "$psiconv_major_found" -eq "$psiconv_major_req" &&
		     test "$psiconv_minor_found" -gt "$psiconv_minor_req"; then
			psion_deps="yes"
		elif test "$psiconv_major_found" -eq "$psiconv_major_req" &&
		     test "$psiconv_minor_found" -eq "$psiconv_minor_req" &&
		     test "$psiconv_micro_found" -ge "$psiconv_micro_req"; then
			psion_deps="yes"
		fi
	fi
fi

if test "$enable_psion" = "yes" || \
   test "$psion_deps" = "yes"; then

if test "$enable_psion_builtin" = "yes"; then
AC_MSG_ERROR([psion plugin: static linking not supported])
fi

AC_MSG_CHECKING([for psiconv >= ${psiconv_major_req}.${psiconv_minor_req}.${psiconv_micro_req}])
if test "$psion_deps" = "yes"; then
	AC_MSG_RESULT([version ${psiconv_major_found}.${psiconv_minor_found}.${psiconv_micro_found} (ok)])
	PSION_CFLAGS=`$psiconvconfig --cflags`
	PSION_LIBS=`$psiconvconfig --libs`
else
	AC_MSG_ERROR([version ${psiconv_major_found}.${psiconv_minor_found}.${psiconv_micro_found} (too old!)])
fi

test "$enable_psion" = "auto" && PLUGINS="$PLUGINS psion"

PSION_CFLAGS="$PSION_CFLAGS $PNG_CFLAGS "'${PLUGIN_CFLAGS}'
PSION_LIBS="$PSION_LIBS $PNG_LIBS -lgsf-1 "'${PLUGIN_LIBS}'

fi

AC_SUBST([PSION_CFLAGS])
AC_SUBST([PSION_LIBS])

