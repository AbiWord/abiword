
pdf_pkgs="$gsf_req"
pdf_deps="no"

PDF_CFLAGS=
PDF_LIBS=

if test "$enable_pdf" != ""; then

PKG_CHECK_EXISTS([ $pdf_pkgs ], 
[
	pdf_deps="yes"
], [
	test "$enable_pdf" = "auto" && AC_MSG_WARN([pdf plugin: dependencies not satisfied - $pdf_pkgs])
])

fi

if test "$enable_pdf" = "yes" || \
   test "$pdf_deps" = "yes"; then

PKG_CHECK_MODULES(PDF,[ $pdf_pkgs ])

test "$enable_pdf" = "auto" && PLUGINS="$PLUGINS pdf"

PDF_CFLAGS="$PDF_CFLAGS "'${PLUGIN_CFLAGS}'
PDF_LIBS="$PDF_LIBS "'${PLUGIN_LIBS}'

if test "$enable_pdf_builtin" = "yes"; then
	PDF_CFLAGS="$PDF_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([PDF_CFLAGS])
AC_SUBST([PDF_LIBS])

