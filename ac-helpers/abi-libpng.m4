# start: abi/ac-helpers/abi-libpng.m4
# 
# Copyright (C) 2002 Francis James Franklin
# Copyright (C) 2002 AbiSource, Inc
# Copyright (C) 2001 Sam Tobin-Hochstadt
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
# Usage: ABI_LIBPNG

AC_DEFUN([ABI_LIBPNG],[

dnl Check for libpng
dnl Supports:
dnl  *  libpng in system library locations
dnl  *  Bundled libpng if a directory name argument is passed

abi_found_libpng="no"

ABI_LIBPNG_DIR=""
AC_ARG_WITH(libpng,[  --with-libpng=DIR     use libpng in DIR],[
	if test "$withval" = "no"; then
		AC_MSG_ERROR([* * * libpng is required by AbiWord * * *])
        elif test "$withval" = "yes"; then
		abi_libpng=check
        elif test "$withval" = "peer"; then
		abi_libpng=peer
	else
		abi_libpng=sys
		ABI_LIBPNG_DIR="$withval"
        fi
],[	abi_libpng=check
])

if test $abi_libpng = check; then
	PKG_CHECK_MODULES(LIBPNG,libpng12,[
		abi_libpng=pkg
		abi_png=sys
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS libpng12"
	],[	abi_libpng=check
	])
fi

if test $abi_libpng = peer; then
	abi_png=peer
elif test $abi_libpng != pkg; then
	if test $abi_libpng = sys; then
		_abi_cppflags="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $ZLIB_CFLAGS -I$ABI_LIBPNG_DIR/include"
	fi
	AC_CHECK_HEADER(png.h,[
		abi_png=sys
	],[	if test $abi_libpng = sys; then
			AC_MSG_ERROR([* * * libpng not found in system location * * *])
		fi
		abi_png=peer
	])
	if test $abi_libpng = sys; then
		CPPFLAGS="$_abi_cppflags"
	fi
fi

LIBPNG_PEERDIR="`cd ..; pwd`/libpng"

if test $abi_png = peer; then
	AC_MSG_CHECKING(for libpng in peer directory)
	if test -d ../libpng; then
		if test -r ../libpng/pngconf.h; then
			AC_MSG_RESULT(yes)
		else
			AC_MSG_RESULT(no)
			AC_MSG_ERROR([unable to use peer libpng - libpng/pngconf.h not found])
		fi
	else
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([unable to use libpng - no peer found])
	fi

	abi_libpng_message="peer libpng"
	LIBPNG_CFLAGS='-I$(top_srcdir)/../libpng'
	LIBPNG_LIBS='$(top_srcdir)/../libpng/libpng.a'

	PEERDIRS="${PEERDIRS} ${LIBPNG_PEERDIR}"
	PEERS="${PEERS} libpng"
else
	if test $abi_libpng = sys; then
		abi_libpng_message="libpng in -L$ABI_LIBPNG_DIR/lib -lpng"
		LIBPNG_CFLAGS="-I$ABI_LIBPNG_DIR/include"
		LIBPNG_LIBS="-L$ABI_LIBPNG_DIR/lib -lpng"
	elif test $abi_libpng = pkg; then
		abi_libpng_message="libpng in $LIBPNG_LIBS"
	else
		abi_libpng_message="libpng in -lpng"
		LIBPNG_CFLAGS=""
		LIBPNG_LIBS="-lpng"
	fi
fi

AC_SUBST(LIBPNG_PEERDIR)

AC_SUBST(LIBPNG_CFLAGS)
AC_SUBST(LIBPNG_LIBS)

])
# 
# end: abi/ac-helpers/abi-libpng.m4
# 
