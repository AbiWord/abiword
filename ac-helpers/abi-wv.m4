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

_abi_wv_warning=no

AC_ARG_WITH(sys_wv,[  --with-sys-wv    Use system libwv],[
	abi_sys_wv="$withval"
],[	abi_sys_wv=no
])

if test "$ABI_NEED_WV" = "yes"; then

if test "x$abi_sys_wv" != "xno"; then
	if test "x$abi_epath" = "xyes"; then
		AC_MSG_ERROR([* * * system wv? I was expecting to build peer wv... * * *])
	fi

# check for a shared install

	if test "x$abi_sys_wv" != "xyes"; then
		AC_PATH_PROG(WVLIBCFG,wv-libconfig,,["$abi_sys_wv"/bin:$PATH])
	else
		AC_PATH_PROG(WVLIBCFG,wv-libconfig,,[$PATH])
	fi
	if test "x$WVLIBCFG" = "x"; then
		AC_MSG_WARN([* * * Can't find wv-libconfig, so I'm just going to guess what libs I need. * * *])
		abi_wv_libs="-lwv -lpng -lz"
	else
		abi_wv_libs=`$WVLIBCFG`
	fi
	AC_CHECK_LIB(wv,wvInitParser,,[
		AC_MSG_WARN([* * * unable to link against libwv. * * *])
		_abi_wv_warning=yes
	],$abi_wv_libs)
	AC_CHECK_HEADER(wv.h,        ,[
		AC_MSG_WARN([* * * unable to find wv.h * * *])
		_abi_wv_warning=yes
	])
	WV_CFLAGS=""
	WV_LIBS="$abi_wv_libs"
	WV_PEERDIR=""

	abi_wv_message="wv in $abi_wv_libs"
else

# otherwise, use the sources given as an argument.  [ this means the
# peer dir for abi ]

	AC_MSG_CHECKING(for wv)
	if test "x$1" != "x" && test -d "$1"; then
		_abi_wv_pdir="$1"
		abi_wv_path=`cd $_abi_wv_pdir; pwd`
		AC_MSG_RESULT($abi_wv_path)
	else
		AC_MSG_ERROR([* * * wv was not found - I looked for it in "$1" * * *])
	fi
	WV_CFLAGS="-I$abi_wv_path"

	if test "x$abi_epath" = "xyes"; then
		WV_LIBS="-L\$(top_builddir)/../wv -lwv"
	else
		WV_LIBS="\$(top_builddir)/../wv/libwv.a"
	fi

	abi_wv_message="supplied wv in $abi_wv_path"

        PEERDIRS="${PEERDIRS} $_abi_wv_pdir"
	PEERS="${PEERS} wv"
fi

AC_DEFINE(HAVE_WV, 1, [ Define if you have wv ])

else
# Abi doesn't need wv...
# 
abi_sys_wv=irrelevant
WV_CFLAGS=""
WV_LIBS=""
fi

AM_CONDITIONAL(LOCAL_WV,[test "x$abi_sys_wv" = "xno"])
AC_SUBST(WV_CFLAGS)
AC_SUBST(WV_LIBS)

])

AC_DEFUN([ABI_WV_WARNING],[

echo ""
echo "WARNING: building against wv as a system library is generally not recommended."
echo "         Compatibility of abi's source with the concurrent wv sources is"
echo "         maintained, and wv itself is developed in AbiSource's CVS repository."
echo "         "
echo "         No doubt you have your reasons; I'll trust you to figure out the"
echo "         awkward dependencies. If you have the corresponding wv sources handy,"
echo "         try adding the source tree to header path, e.g.:"
echo "         "
echo "             ./configure ... CPPFLAGS=\"-I/home/me/src/wv-0.7.5\""
echo "         "
echo "         Note: AbiWord-1.0.x requires a different version of libwv"
echo ""

])
