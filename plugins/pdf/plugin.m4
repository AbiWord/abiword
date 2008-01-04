
pdf_pkgs="$gsf_req"

PDF_CFLAGS=
PDF_LIBS=

if test "$enable_pdf" == "yes"; then

AC_PATH_PROG(PDFTOABW, pdftoabw)
if test "$PDFTOABW" == ""; then
	AC_MSG_ERROR([program pdftoabw not found in path])
fi

PKG_CHECK_MODULES(PDF,[ $pdf_pkgs ])

PDF_CFLAGS="$PDF_CFLAGS "'${WP_CPPFLAGS}'
PDF_LIBS="$PDF_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([PDF_CFLAGS])
AC_SUBST([PDF_LIBS])

