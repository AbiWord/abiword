# start: abi/ac-helpers/abi-sugar.m4
# 
# Copyright (C) 2002-2003 AbiSource, Inc
# 
# This file is free software; you may copy and/or distribute it with
# or without modifications, as long as this notice is preserved.
# This software is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.
#
# The above license applies to THIS FILE ONLY, the abiword code
# itself may be copied and distributed under the terms of the GNU
# GPL, see COPYING for more details
#
# This file detects which of the AbiWord platforms we are currently
# building on.  The detection logic in question is mostly by Jeff
# Hostetler, and is taken from the original AbiWord build system.  
#
# Usage: ABI_SUGAR

AC_DEFUN([ABI_SUGAR], [

dnl Build for sugar framework on the OLPC

sugar=false

SUGAR_CFLAGS=""
SUGAR_LIBS=""

AC_ARG_ENABLE(sugar,[  --enable-sugar    Build for sugar framework/OLPC ],[

	if test "x$enableval" = "xyes"; then
		if test "$PLATFORM" != unix; then
			AC_MSG_ERROR([sorry: --enable-sugar supported only on UNIX platforms])
		fi
		sugar=true
	fi
])

if test "$sugar" = true ; then
	AM_PATH_PYTHON
	SUGAR_CFLAGS="$SUGAR_CFLAGS -DHAVE_SUGAR=1"
	SUGAR_LIBS="$SUGAR_LIBS"
fi

AC_SUBST(SUGAR_CFLAGS)
AC_SUBST(SUGAR_LIBS)
	
AM_CONDITIONAL(WITH_SUGAR, test "x$sugar" = "xtrue")

])
# 
# end: abi/ac-helpers/abi-sugar.m4
# 
