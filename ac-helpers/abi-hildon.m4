# start: abi/ac-helpers/abi-hildon.m4
# 
# Copyright (C) 2002-2003 AbiSource, Inc
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
# Usage: ABI_HILDON_QUICK

AC_DEFUN([ABI_HILDON_QUICK], [

dnl Quick&Easy HILDON Detection

hildon=false

HILDON_CFLAGS=""
HILDON_LIBS=""

AC_ARG_ENABLE(hildon,[  --enable-hildon    Turn on hildon ],[
	if test "x$enableval" = "xyes"; then
		if test "$PLATFORM" != unix; then
			AC_MSG_ERROR([sorry: --enable-hildon supported only on UNIX platforms])
		fi
		hildon=true
	fi
])

if test "$hildon" = true ; then
PKG_CHECK_MODULES(HILDON, hildon-libs dbus-1, HAVE_HILDON=yes,HAVE_HILDON=no)
	PKG_CHECK_MODULES(HILDON,[
		hildon-libs,
		dbus-1,
		libosso
	])
	HILDON_CFLAGS="$HILDON_CFLAGS -DHAVE_HILDON=1 -DEMBEDDED_TARGET=1"
	HILDON_LIBS="$HILDON_LIBS"
fi

AC_SUBST(HILDON_CFLAGS)
AC_SUBST(HILDON_LIBS)
	
AM_CONDITIONAL(WITH_HILDON, test "x$hildon" = "xtrue")

])
# 
# end: abi/ac-helpers/abi-hildon.m4
# 
