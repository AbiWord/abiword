# Copyright (C) 2001 Sam Tobin-Hochstadt
# This file is free software; you may copy and/or distribute it with
# or without modifications, as long as this notice is preserved.
# This software is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.

# The above license applies to THIS FILE ONLY, the abiword code
# itself may be copied and distributed under the terms of the GNU
# GPL, see COPYING for more details

# Check for Wv library.
# Supports:
#  *  wv in -lwv
#  *  Bundled wv if a directory name argument is passed
#
# Hacked from the abi-xml-parser.m4 code
#
# Usage: 
#  ABI_WV
# or
#  ABI_WV(wv-dir)

AC_DEFUN([ABI_WV], [

if test "$ABI_NEED_WV" = "yes"; then

abi_found_wv="no"

# check for a shared install

if test "$abi_found_wv" = "no"; then
	echo "checking for wv"
	AC_CHECK_LIB(wv, wvInitParser,
		WV_LIBS="-lwv" abi_found_wv="yes"
		)
fi

# check for the header file

if test "$abi_found_wv" = "yes"; then
	AC_CHECK_HEADER(wv.h, 
	[abi_found_wvincs="yes"])
	if test "$abi_found_wvincs" = "yes"; then
		WV_CFLAGS=""
		abi_wv_message="wv in $abi_wv_libs"
	else 
		#AC_MSG_WARN([wv library found but header file missing])
		abi_found_wv="no"
		WV_LIBS=""
	fi
fi


# otherwise, use the sources given as an argument.  [ this means the
# peer dir for abi ]

if test "$abi_found_wv" = "no"; then
    if test "x$1" != "x" && test -d "$1"; then
	abspath=`cd $1; pwd`
	WV_LIBS="${abspath}/libwv.a"
	WV_CFLAGS="-I${abspath}/"
	AC_MSG_RESULT(using supplied wv library)	
	AC_DEFINE(HAVE_WV, 1, [ Define if you have wv ])
	abi_wv_message="supplied wv in ${abspath}"
	WV_PEERDIR=${abspath}
        PEERDIRS="${PEERDIRS} ${WV_PEERDIR}"
	PEERS="${PEERS} `basename ${abspath}`"
    else
	AC_MSG_ERROR([ wv was not found ])
    fi

fi

fi

AM_CONDITIONAL(LOCAL_WV, test "$local_wv" = "true")
AC_SUBST(WV_CFLAGS)
AC_SUBST(WV_LIBS)
AC_SUBST(WV_PEERDIR)

])
