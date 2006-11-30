# start: abi/ac-helpers/abi-xft.m4
# 
# Copyright (C) 2002 AbiSource, Inc
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
# Usage: ABI_XFT_QUICK

AC_DEFUN([ABI_XFT_QUICK], [

dnl Quick&Easy Xft2 Detection

XFT_CFLAGS=""
XFT_LIBS=""

xft=true

if test "$PLATFORM" != unix; then
	xft=false
fi

xft_modules='xft >= 2.0 fontconfig >= 1.0'

if test "x$xft" = "xtrue" ; then
	PKG_CHECK_MODULES(XFT,[ $xft_modules ], 
	[
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS $xft_modules"
	])
	XFT_CFLAGS="$XFT_CFLAGS"
fi

AC_SUBST(XFT_CFLAGS)
AC_SUBST(XFT_LIBS)
	
AM_CONDITIONAL(WITH_XFT, test "x$xft" = "xtrue")

])
# 
# end: abi/ac-helpers/abi-xft.m4
# 
