# start: abi/ac-helpers/abi-lib.m4
# 
# Copyright (C) 2006 Robert Staudinger <robert.staudinger@gmail.com>
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
# Usage: ABI_LIB

AC_DEFUN([ABI_LIB], [

dnl Build library version of abiword

libabiword=false
abi_lib_message="no"

AC_ARG_ENABLE(libabiword,[  --enable-libabiword    Build libabiword ],[

	if test "x$enableval" = "xyes"; then
		if test "$PLATFORM" != unix; then
			AC_MSG_ERROR([sorry: --enable-libabiword supported only on UNIX platforms])
		fi
		libabiword=true
		abi_lib_message="yes"
	fi
])

if test "$libabiword" = true ; then
	CFLAGS="$CFLAGS -fPIC"
	CXXFLAGS="$CXXFLAGS -fPIC"
fi

AM_CONDITIONAL(WITH_LIBABIWORD, test "x$libabiword" = "xtrue")

])
# 
# end: abi/ac-helpers/abi-lib.m4
# 
