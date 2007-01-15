# start: abi/ac-helpers/abi-zlib.m4
# 
# Copyright (C) 2002 Francis James Franklin
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
# Usage: ABI_ZLIB

AC_DEFUN([ABI_ZLIB],[

dnl Check for zlib
dnl Supports:
dnl  *  zlib in system library locations
dnl  *  Bundled zlib if a directory name argument is passed

abi_found_zlib="no"

ABI_ZLIB_DIR=""
AC_ARG_WITH(zlib,[  --with-zlib=DIR       use zlib in DIR],[
	if [ test "$withval" = "no" ]; then
		AC_MSG_ERROR([* * * zlib is required by AbiWord * * *])
        elif [ test "$withval" = "yes" ]; then
		abi_zlib=check
        elif [ test "$withval" = "peer" ]; then
		abi_zlib=peer
	else
		abi_zlib=sys
		ABI_ZLIB_DIR="$withval"
        fi
],[	abi_zlib=check
])

if test $abi_zlib = peer; then
	abi_z=peer
else
	if test $abi_zlib = sys; then
		_abi_cppflags="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS -I$ABI_ZLIB_DIR/include"
	fi
	AC_CHECK_HEADER(zlib.h,[
		abi_z=sys
	],[	if test $abi_zlib = sys; then
			AC_MSG_ERROR([* * * zlib not found in system location * * *])
		fi
		abi_z=peer
	])
	if test $abi_zlib = sys; then
		CPPFLAGS="$_abi_cppflags"
	fi
fi

dnl ZLIB_PEERDIR="`cd ..; pwd`/zlib"

if test $abi_z = peer; then
	AC_MSG_CHECKING(for zlib in peer directory)
	if test -d ../zlib; then
		if test -r ../zlib/libz.a; then
			AC_MSG_RESULT(yes)
		else
			AC_MSG_RESULT(no)
			AC_MSG_ERROR([unable to use peer zlib - zlib/libz.a not found])
		fi
	else
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([unable to use zlib - no peer found])
	fi

	abi_zlib_message="peer zlib"
	ZLIB_CFLAGS='-I$(top_srcdir)/../zlib'
	ZLIB_LIBS='$(top_srcdir)/../zlib/libz.a'

dnl	PEERDIRS="${PEERDIRS} ${ZLIB_PEERDIR}"
dnl	PEERS="${PEERS} zlib"

else
	if test $abi_zlib = sys; then
		abi_zlib_message="zlib in -L$ABI_ZLIB_DIR/lib -lz"
		ZLIB_CFLAGS="-I$ABI_ZLIB_DIR/include"
		ZLIB_LIBS="-L$ABI_ZLIB_DIR/lib -lz"
	else
		abi_zlib_message="zlib in -lz"
		ZLIB_CFLAGS=""
		ZLIB_LIBS="-lz"
	fi
fi

dnl AC_SUBST(ZLIB_PEERDIR)

AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)

])
# 
# end: abi/ac-helpers/abi-zlib.m4
# 
