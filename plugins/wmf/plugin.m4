
AC_ARG_WITH([libwmf-config],
	[AS_HELP_STRING([--with-libwmf-config=DIR], [use libwmf-config in DIR])],
[
	AC_PATH_PROG(libwmfconfig, libwmf-config, , "$withval")
], [
	AC_PATH_PROG(libwmfconfig, libwmf-config)
])

# The required libwmf version, as reported by libwmf-config
libwmf_major_req=0
libwmf_minor_req=2
libwmf_micro_req=8
wmf_deps="no"

if test "$enable_wmf" != ""; then

	if test "$libwmfconfig" = ""; then
		if test "$enable_wmf" = "yes"; then
		  AC_MSG_ERROR([wmf plugin: program libwmf-config not found in path])
		else
		  AC_MSG_WARN([wmf plugin: program libwmf-config not found in path])
		fi
	else
		IFS_old="$IFS"
		IFS='.'
		set -- `$libwmfconfig --version`
		libwmf_major_found="${1}"
		libwmf_minor_found="${2}"
		libwmf_micro_found="${3}"
		IFS="$IFS_old"
		if test "$libwmf_major_found" -gt "$libwmf_major_req"; then
			wmf_deps="yes"
		elif test "$libwmf_major_found" -eq "$libwmf_major_req" &&
		     test "$libwmf_minor_found" -gt "$libwmf_minor_req"; then
			wmf_deps="yes"
		elif test "$libwmf_major_found" -eq "$libwmf_major_req" &&
		     test "$libwmf_minor_found" -eq "$libwmf_minor_req" &&
		     test "$libwmf_micro_found" -ge "$libwmf_micro_req"; then
			wmf_deps="yes"
		fi
	fi
fi

if test "$enable_wmf" = "yes" || \
   test "$wmf_deps" = "yes"; then

if test "$enable_wmf_builtin" = "yes"; then
AC_MSG_ERROR([wmf plugin: static linking not supported])
fi

AC_MSG_CHECKING([for libwmf >= ${libwmf_major_req}.${libwmf_minor_req}.${libwmf_micro_req}])
if test "$wmf_deps" = "yes"; then
	AC_MSG_RESULT([version ${libwmf_major_found}.${libwmf_minor_found}.${libwmf_micro_found} (ok)])
	WMF_CFLAGS=`$libwmfconfig --cflags`
	WMF_LIBS=`$libwmfconfig --libs`
else
	AC_MSG_ERROR([version ${libwmf_major_found}.${libwmf_minor_found}.${libwmf_micro_found} (too old!)])
fi

test "$enable_wmf" = "auto" && PLUGINS="$PLUGINS wmf"

WMF_CFLAGS="$WMF_CFLAGS "'${PLUGIN_CFLAGS}'
WMF_LIBS="$WMF_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([WMF_CFLAGS])
AC_SUBST([WMF_LIBS])

