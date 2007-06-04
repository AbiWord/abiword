# start: abi/ac-helpers/abi-gucharmap.m4
# 
# Copyright (C) 2003 Dom Lachowicz
# Copyright (C) 2003 AbiSource, Inc
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
# Usage: ABI_GUCHARMAP

dnl Check for gucharmap

AC_DEFUN([ABI_GUCHARMAP], [

test_cmap=true
have_cmap=false

GUCHARMAP_CFLAGS=""
GUCHARMAP_LIBS=""

AC_ARG_ENABLE(gucharmap,[  --disable-gucharmap  Turn off gucharmap ], [
	if test "x$enableval" = "xno"; then
		test_cmap=false
	fi
])

if test "x$test_cmap" = "xtrue" ; then
	PKG_CHECK_MODULES(GUCHARMAP,[gucharmap >= 1.4], 
	[
		have_cmap=true
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS gucharmap >= 1.4"
	], [
		have_cmap=false
	])
fi

if test "x$have_cmap" = "xtrue" ; then
	GUCHARMAP_CFLAGS="$GUCHARMAP_CFLAGS -DWITH_GUCHARMAP"
fi

AC_SUBST(GUCHARMAP_CFLAGS)
AC_SUBST(GUCHARMAP_LIBS)

AM_CONDITIONAL(WITH_GUCHARMAP, test "x$cmap" = "xtrue")

])

#
# end: abi/ac-helpers/abi-gucharmap.m4
#
