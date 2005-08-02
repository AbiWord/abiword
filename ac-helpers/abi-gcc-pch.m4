# Checks whether to use gcc's pch support

AC_DEFUN([ABI_GCC_PCH],[

	AC_ARG_ENABLE(gcc-pch,
		[  --enable-gcc-pch[=yes no auto]       use gcc4 pch support (default=auto)],
		enable_gcc_pch=$enableval,enable_gcc_pch=auto)

	AC_MSG_CHECKING(whether to use gcc precompiled headers with $CXX)
	if test "x$GCC" = "xyes" ; then

		_abi_gcc_version=`$CXX --version | grep -m 1 GCC |  cut -f 3 -d " "`
		_abi_gcc_major=`echo $_abi_gcc_version | cut -f 1 -d "."`

		if [[ "$_abi_gcc_major" -ge "4" ]] ; then
			if test "x$enable_gcc_pch" = "xyes" ; then
				AC_MSG_RESULT(yes)
				abi_gcc_pch="yes"
			elif test "x$enable_gcc_pch" = "xauto" ; then
				AC_MSG_RESULT(yes)
				abi_gcc_pch="yes"
			else
				AC_MSG_RESULT(no)
				abi_gcc_pch="no"
			fi
		else
			AC_MSG_RESULT(no)
			abi_gcc_pch="no"
		fi
	else
		abi_gcc_pch="no"				
	fi

	AM_CONDITIONAL(USE_GCC_PCH, test "x$abi_gcc_pch" = "xyes")
])
