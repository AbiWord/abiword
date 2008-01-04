
# gsf pulls in libxml, so we are ok
mht_pkgs="$gsf_req"

if test "$enable_mht" == "yes"; then

#
# Optional packages
#

AC_ARG_WITH([inter7eps], 
	[AS_HELP_STRING([--with-inter7eps], [MHT plugin: support multipart html using the inter7 EPS library])], 
[
	mht_cv_inter7eps="$withval"
],[
	mht_cv_inter7eps="auto"
])

AC_ARG_WITH([libtidy], 
	[AS_HELP_STRING([--with-libtidy], [MHT plugin: clean up HTML before importing using libtidy])], 
[
	mht_cv_libtidy="$withval"
],[
	mht_cv_libtidy="auto"
])

#
# Tests
#

AC_CHECK_HEADERS([eps/eps.h],
[
	inter7eps_found="yes"
], [
	inter7eps_found="no"
])

AC_CHECK_HEADERS([tidy/tidy.h],
[
	libtidy_found="yes"
], [
	libtidy_found="no"
])

#
# Settings
#

if test "$mht_cv_inter7eps" == "yes" &&
   test "$inter7eps_found" == "no"; then
	AC_MSG_ERROR([MHT plugin: error - inter7 EPS headers not found])
elif test "$mht_cv_inter7eps" == "auto"; then
	mht_cv_inter7eps="$inter7eps_found"
fi
if test "$mht_cv_inter7eps" == "yes"; then
	MHT_OPT_LIBS="$MHT_OPT_LIBS -leps"
fi

if test "$mht_cv_libtidy" == "yes" &&
   test "$libtidy_found" == "no"; then
	AC_MSG_ERROR([MHT plugin: error - libtidy headers not found])
elif test "$mht_cv_libtidy" == "auto"; then
	mht_cv_libtidy="$libtidy_found"
fi
if test "$mht_cv_libtidy" == "yes"; then
	MHT_OPT_LIBS="$MHT_OPT_LIBS -ltidy"
fi

PKG_CHECK_MODULES(MHT,[ $mht_pkgs ])

MHT_CFLAGS="$MHT_CFLAGS "'${WP_CPPFLAGS}'
MHT_LIBS="$MHT_LIBS $MHT_OPT_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([MHT_CFLAGS])
AC_SUBST([MHT_LIBS])

# TODO we depend on libxml2, so get rid of alternatives
AM_CONDITIONAL([ABI_XHTML_XML2], test /bin/true)
AM_CONDITIONAL([ABI_XHTML_MHT], test "$mht_cv_inter7eps" == "yes")
AM_CONDITIONAL([ABI_XHTML_TIDY], test "$mht_cv_libtidy" == "yes")
