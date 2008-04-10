
pdf_pkgs="$gsf_req"

PDF_CFLAGS=
PDF_LIBS=

if test "$enable_pdf" == "yes"; then

PKG_CHECK_MODULES(PDF,[ $pdf_pkgs ])

PDF_CFLAGS="$PDF_CFLAGS "'${PLUGIN_CFLAGS}'
PDF_LIBS="$PDF_LIBS "'${PLUGIN_LIBS}'

if test "$enable_pdf_builtin" == "yes"; then
	PDF_CFLAGS="$PDF_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([PDF_CFLAGS])
AC_SUBST([PDF_LIBS])

