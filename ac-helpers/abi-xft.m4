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

AC_ARG_ENABLE(xft,[  --enable-xft    Turn on xft ],[
	case "${enableval}" in
	 yes)	if test "$PLATFORM" = unix; then
			xft=true
		else
			AC_MSG_ERROR([sorry: --enable-xft supported only on UNIX platforms])
			xft=false
		fi
		;;
	  no)	xft=false ;;
	   *)	AC_MSG_ERROR(bad value ${enableval} for --enable-xft) ;;
	esac
],[	xft=false
])

if test "x$xft" = "xtrue" ; then
    AC_MSG_CHECKING(for Xft >= 2.0.0)
    dnl We need the "%d" in order not to get e-notation on hpux.
    vers=`xft-config --version | awk 'BEGIN { FS = "."; } { printf "%d", ([$]1 * 1000 + [$]2) * 1000 + [$]3;}'`
    if test "$vers" -ge 2000000; then
        AC_MSG_RESULT(found)
    else
        AC_MSG_RESULT(You need at least Xft 2.0.0: disabling xft)
        xft=false
    fi
fi

if test "x$xft" = "xtrue" ; then
	XFT_CFLAGS="`xft-config --cflags` -DUSE_XFT=1"
	XFT_LIBS="`xft-config --libs`"
fi

AC_SUBST(XFT_CFLAGS)
AC_SUBST(XFT_LIBS)
	
AM_CONDITIONAL(WITH_XFT, test "x$xft" = "xtrue")

])
# 
# end: abi/ac-helpers/abi-xft.m4
# 
