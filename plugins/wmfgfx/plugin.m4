
# The required libwmf version, as reported by libwmf-config
libwmf_major_req=0
libwmf_minor_req=2
libwmf_micro_req=8

WMFGFX_CFLAGS=
WMFGFX_LIBS=

if test "$enable_wmfgfx" == "yes"; then

AC_ARG_WITH([libwmf-config],
	[AS_HELP_STRING([--with-libwmf-config=DIR], [use libwmf-config in DIR])],
[
	AC_PATH_PROG(libwmfconfig, libwmf-config, , "$withval")
], [
	AC_PATH_PROG(libwmfconfig, libwmf-config)
])

if test "$libwmfconfig" == ""; then
	AC_MSG_ERROR([program libwmf-config not found in path])
else
	AC_MSG_CHECKING([for libwmf >= ${libwmf_major_req}.${libwmf_minor_req}.${libwmf_micro_req}])
	IFS_old="$IFS"
	IFS='.'
	set -- `$libwmfconfig --version`
	libwmf_major_found="${1}"
	libwmf_minor_found="${2}"
	libwmf_micro_found="${3}"
	IFS="$IFS_old"
	if test "$libwmf_major_found" -gt "$libwmf_major_req"; then
		libwmf_found="yes"
	elif test "$libwmf_major_found" -eq "$libwmf_major_req" &&
	     test "$libwmf_minor_found" -gt "$libwmf_minor_req"; then
		libwmf_found="yes"
	elif test "$libwmf_major_found" -eq "$libwmf_major_req" &&
	     test "$libwmf_minor_found" -eq "$libwmf_minor_req" &&
	     test "$libwmf_micro_found" -ge "$libwmf_micro_req"; then
		libwmf_found="yes"
	else
		libwmf_found="no"
	fi
	if test "$libwmf_found" == "yes"; then
		AC_MSG_RESULT([version ${libwmf_major_found}.${libwmf_minor_found}.${libwmf_micro_found} (OK)])
		WMF_CFLAGS=`$libwmfconfig --cflags`
		WMF_LIBS=`$libwmfconfig --libs`
	else
		AC_MSG_ERROR([version ${libwmf_major_found}.${libwmf_minor_found}.${libwmf_micro_found} (Too old!)])
	fi
fi

WMFGFX_CFLAGS="$WMFGFX_CFLAGS "'${PLUGIN_CFLAGS}'
WMFGFX_LIBS="$WMFGFX_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([WMFGFX_CFLAGS])
AC_SUBST([WMFGFX_LIBS])

