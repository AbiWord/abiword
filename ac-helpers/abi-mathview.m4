# test for MathView
AC_DEFUN([ABI_MATHVIEW],[

	abi_mathview=check

	AC_ARG_ENABLE(mathview,[  --disable-mathview    don't use GtkMathView ],[
        	if test "x$enableval" = "xno"; then
                	abi_mathview=disabled
        	fi
	])

	if test $abi_mathview = check; then
		PKG_CHECK_MODULES(MATHVIEW, mathview-libxml2 >= 0.6.5, [
			abi_mathview=yes
		],[	abi_mathview=no
		])
	fi

	abi_mathview_message="$abi_mathview"
	AM_CONDITIONAL(HAVE_MATHVIEW, test $abi_mathview = yes)
])

AC_DEFUN([ABI_HASHMAP],[

	AC_LANG_PUSH(C++)
	AC_CHECK_HEADERS(hash_map)
	AC_CHECK_HEADERS(ext/hash_map)
	AC_LANG_POP(C++)

])

