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

# Check for libpng
# Supports:
#  *  libpng in system library locations
#  *  Bundled libpng if a directory name argument is passed
#
# Hacked from the abi-xml-parser.m4 code
#
# Usage: 
#  ABI_LIBPNG
# or
#  ABI_LIBPNG(libpng-dir)

AC_DEFUN([ABI_LIBPNG], [

if test "$ABI_NEED_LIBPNG" = "yes"; then

abi_found_libpng="no"

# check for a shared install

if test "$abi_found_libpng" = "no"; then
	echo "checking for libpng"
	AC_CHECK_LIB(png, png_create_info_struct,
		abi_libpng_libs="-lpng" abi_found_libpng="yes"
		)
fi

# check for the header file

if test "$abi_found_libpng" = "yes"; then
	AC_CHECK_HEADER(png.h, 
	[abi_found_libpngincs="yes"])
	if test "$abi_found_libpngincs" = "yes"; then
		LIBPNG_CFLAGS=""
		LIBPNG_LIBS="$abi_libpng_libs"
		abi_libpng_message="libpng in $abi_libpng_libs"
	else 
		#AC_MSG_WARN([png library found but header file missing])
		abi_found_libpng="no"
	fi
fi


# otherwise, use the sources given as an argument.  [ this means the
# peer dir for abi ]

if test "$abi_found_libpng" = "no"; then
    if test "x$1" != "x" && test -d "$1"; then
	abspath=`cd $1; pwd`
	LIBPNG_LIBS="${abspath}/libpng.a"
	LIBPNG_CFLAGS="-I${abspath}/"
	AC_MSG_RESULT(using supplied png library)	
	AC_DEFINE(HAVE_LIBPNG, 1, [ Define if you have libpng ])
	abi_libpng_message="supplied libpng in ${abspath}"
	LIBPNG_PEERDIR=${abspath}
        PEERDIRS="${PEERDIRS} ${LIBPNG_PEERDIR}"
	PEERS="${PEERS} `basename ${abspath}`"
    else
	AC_MSG_ERROR([ libpng was not found ])
    fi

fi

fi

AM_CONDITIONAL(LOCAL_LIBPNG, test "$local_libpng" = "true")
AC_SUBST(LIBPNG_CFLAGS)
AC_SUBST(LIBPNG_LIBS)
AC_SUBST(LIBPNG_PEERDIR)

])
