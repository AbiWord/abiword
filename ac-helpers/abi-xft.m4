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

FREETYPE_CFLAGS=""
FREETYPE_LIBS=""

AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

XFT_CFLAGS=""
XFT_LIBS=""

xft=true

AC_ARG_ENABLE(xft,[  --enable-xft    Turn on xft ],[
	if test "x$enableval" = "xno"; then
		xft=false
	fi
],[	xft=false
])

if test "$PLATFORM" = unix; then
	if test $xft = false; then
		AC_MSG_WARN([* * * building on *NIX w/o Xft2 is not supported * * *])
	fi
else
	xft=false
fi

if test "x$xft" = "xtrue" ; then
	PKG_CHECK_MODULES(XFT,[xft >= 2.0 fontconfig >= 1.0])
fi

AC_SUBST(XFT_CFLAGS)
AC_SUBST(XFT_LIBS)
	
AM_CONDITIONAL(WITH_XFT, test "x$xft" = "xtrue")

])
# 
# end: abi/ac-helpers/abi-xft.m4
# 
