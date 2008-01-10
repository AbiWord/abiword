
pdf_pkgs="$gsf_req"

PDF_CFLAGS=
PDF_LIBS=

if test "$enable_pdf" == "yes"; then

PKG_CHECK_MODULES(PDF,[ $pdf_pkgs ])

PDF_CFLAGS="$PDF_CFLAGS "'${WP_CPPFLAGS}'
PDF_LIBS="$PDF_LIBS "'${PLUGIN_LIBS}'

fi

AC_SUBST([PDF_CFLAGS])
AC_SUBST([PDF_LIBS])

