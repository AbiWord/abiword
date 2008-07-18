
LATEX_CFLAGS=
LATEX_LIBS=

if test "$enable_latex" == "yes"; then

libxslt_req='libxslt'

# use libxslt if detected
PKG_CHECK_EXISTS([ $libxslt_req ],
[
	abi_cv_libxslt="yes"
], [
	abi_cv_libxslt="no"
])

AM_CONDITIONAL([HAVE_LIBXSLT], test "$abi_cv_libxslt" == "yes")

if test "$abi_cv_libxslt" == "yes"; then
	PKG_CHECK_MODULES(LIBXSLT,[$libxslt_req])
	LATEX_CFLAGS="$LATEX_CFLAGS "'${LIBXSLT_CFLAGS}'" -DHAVE_LIBXSLT"
	LATEX_LIBS="$LATEX_LIBS "'${LIBXSLT_LIBS}'
	AC_SUBST(ABIWORD_XSLTMLDIR, "${ABIWORD_DATADIR}/xsltml")
fi

LATEX_CFLAGS="$LATEX_CFLAGS "'${PLUGIN_CFLAGS}'
LATEX_LIBS="$LATEX_LIBS "'${PLUGIN_LIBS}'

if test "$enable_latex_builtin" == "yes"; then
	LATEX_CFLAGS="$LATEX_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([LATEX_CFLAGS])
AC_SUBST([LATEX_LIBS])

