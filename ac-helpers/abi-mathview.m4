# test for MathView
AC_DEFUN([ABI_MATHVIEW],[

	PKG_CHECK_MODULES(MATHVIEW, mathview-libxml2 >= 0.6.4, [
	dnl do we need to set these?
		abi_mathview=yes
	],[	abi_mathview=no
	])

	abi_mathview_message="$abi_mathview"
	AM_CONDITIONAL(HAVE_MATHVIEW, test $abi_mathview = yes)
])

AC_DEFUN([ABI_HASHMAP],[

	AC_LANG_PUSH(C++)
	AC_CHECK_HEADERS(hash_map)
	AC_CHECK_HEADERS(ext/hash_map)
	AC_LANG_POP(C++)

])

