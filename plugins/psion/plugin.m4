
# The required psiconv version, as reported by psiconv-config
psiconv_major_req=0
psiconv_minor_req=9
psiconv_micro_req=4

PSION_CFLAGS=
PSION_LIBS=

if test "$enable_psion" == "yes"; then

AC_ARG_WITH([psiconv-config],
	[AS_HELP_STRING([--with-psiconv-config=DIR], [use psiconv-config in DIR])],
[
	AC_PATH_PROG(psiconvconfig, psiconv-config, , "$withval")
], [
	AC_PATH_PROG(psiconvconfig, psiconv-config)
])

if test "$psiconvconfig" == ""; then
	AC_MSG_ERROR([program psiconv-config not found in path])
else
	AC_MSG_CHECKING([for psiconv >= ${psiconv_major_req}.${psiconv_minor_req}.${psiconv_micro_req}])
	IFS_old="$IFS"
	IFS='.'
	set -- `$psiconvconfig --version`
	psiconv_major_found="${1}"
	psiconv_minor_found="${2}"
	psiconv_micro_found="${3}"
	IFS="$IFS_old"
	if test "$psiconv_major_found" -gt "$psiconv_major_req"; then
		psiconv_found="yes"
	elif test "$psiconv_major_found" -eq "$psiconv_major_req" &&
	     test "$psiconv_minor_found" -gt "$psiconv_minor_req"; then
		psiconv_found="yes"
	elif test "$psiconv_major_found" -eq "$psiconv_major_req" &&
	     test "$psiconv_minor_found" -eq "$psiconv_minor_req" &&
	     test "$psiconv_micro_found" -ge "$psiconv_micro_req"; then
		psiconv_found="yes"
	else
		psiconv_found="no"
	fi
	if test "$psiconv_found" == "yes"; then
		AC_MSG_RESULT([version ${psiconv_major_found}.${psiconv_minor_found}.${psiconv_micro_found} (OK)])
		PSION_CFLAGS=`$psiconvconfig --cflags`
		PSION_LIBS=`$psiconvconfig --libs`
	else
		AC_MSG_ERROR([version ${psiconv_major_found}.${psiconv_minor_found}.${psiconv_micro_found} (Too old!)])
	fi
fi

PSION_CFLAGS="$PSION_CFLAGS"'${WP_CPPFLAGS}'
PSION_LIBS="$PSION_LIBS"'${PLUGIN_LIBS}'

fi

AC_SUBST([PSION_CFLAGS])
AC_SUBST([PSION_LIBS])

