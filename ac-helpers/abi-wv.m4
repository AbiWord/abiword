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

AC_ARG_WITH(sys_wv,[  --with-sys-wv    Use system libwv],[
	abi_sys_wv="$withval"
],[	abi_sys_wv=no
])

if test "$ABI_NEED_WV" = "yes"; then

if test "x$abi_sys_wv" != "xno"; then

# check for a shared install

	if test "x$abi_sys_wv" != "xyes"; then
		AC_PATH_PROG(WVLIBCFG,wv-libconfig,,["$abi_sys_wv"/bin:$PATH])
	else
		AC_PATH_PROG(WVLIBCFG,wv-libconfig,,[$PATH])
	fi
	if test "x$WVLIBCFG" = "x"; then
		if test "x$abi_sys_wv" != "xyes"; then
			AC_PATH_PROG(LIBWV_CONFIG,libwv-config,,["$abi_sys_wv"/bin:$PATH])
		else
			AC_PATH_PROG(LIBWV_CONFIG,libwv-config,,[$PATH])
		fi
		if test "x$LIBWV_CONFIG" = "x"; then
			AC_MSG_WARN([* * * Can't find wv-libconfig or libwv-config - going to guess what libs I need. * * *])
			abi_wv_libs="-lwv -lpng -lz"
		else
			abi_wv_libs=`$LIBWV_CONFIG --libs`
		fi
	else
		abi_wv_libs=`$WVLIBCFG`
	fi
	AC_CHECK_LIB(wv,wvInitParser,,[AC_MSG_ERROR([* * * Sorry, unable to link against libwv. * * *])],$abi_wv_libs)
	AC_CHECK_HEADER(wv.h,        ,[AC_MSG_ERROR([* * * Sorry, unable to find wv.h * * *])])
	AC_CHECK_HEADER(wvexporter.h,,[AC_MSG_ERROR([* * * Sorry, unable to find wvexporter.h * * *])])
	WV_CFLAGS=""
	WV_LIBS="$abi_wv_libs"
	WV_PEERDIR=""

	abi_wv_message="wv in $abi_wv_libs"
else

# otherwise, use the sources given as an argument.  [ this means the
# peer dir for abi ]

	AC_MSG_CHECKING(for wv)
	if test "x$1" != "x" && test -d "$1"; then
		abspath=`cd $1; pwd`
		AC_MSG_RESULT($abspath)	
	else
		AC_MSG_ERROR([* * * wv was not found - I looked for it in "$1" * * *])
	fi
	WV_CFLAGS="-I${abspath}"
	WV_LIBS="${abspath}/libwv.a"
	WV_PEERDIR="${abspath}"

	abi_wv_message="supplied wv in ${abspath}"

        PEERDIRS="${PEERDIRS} ${WV_PEERDIR}"
	PEERS="${PEERS} `basename ${abspath}`"
fi

AC_DEFINE(HAVE_WV, 1, [ Define if you have wv ])

else
# Abi doesn't need wv...
# 
abi_sys_wv=irrelevant
WV_CFLAGS=""
WV_LIBS=""
WV_PEERDIR=""
fi

AM_CONDITIONAL(LOCAL_WV,[test "x$abi_sys_wv" = "xno"])
AC_SUBST(WV_CFLAGS)
AC_SUBST(WV_LIBS)
AC_SUBST(WV_PEERDIR)

])
