# start: abi/ac-helpers/abi-gnome.m4
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
# Usage: ABI_GNOME_QUICK

AC_DEFUN([ABI_GNOME_QUICK], [

dnl Quick&Easy GNOME Detection

gnome=false

GNOME_CFLAGS=""
GNOME_LIBS=""

AC_ARG_ENABLE(gnome,[  --enable-gnome    Turn on gnome ],[
	if test "x$enableval" = "xyes"; then
		if test "$PLATFORM" != unix; then
			AC_MSG_ERROR([sorry: --enable-gnome supported only on UNIX platforms])
		fi
		gnome=true
	fi
])

if test "$gnome" = true ; then
	PKG_CHECK_MODULES(GNOME,[
		libbonobo-2.0 >= 2.0
		libgnomeui-2.0 >= 2.0
		libgnomeprint-2.2 >= 2.2.1
		libgnomeprintui-2.2 >= 2.2.1
		libglade-2.0 >= 2.0
	])
	GNOME_CFLAGS="$GNOME_CFLAGS -DHAVE_GNOME=1"
	GNOME_LIBS="$GNOME_LIBS"
fi

#GNOME_CFLAGS="-DGNOME_DISABLE_DEPRECATED $GNOME_CFLAGS"
AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GNOME_LIBS)
	
AM_CONDITIONAL(WITH_GNOME, test "x$gnome" = "xtrue")

])
# 
# end: abi/ac-helpers/abi-gnome.m4
# 
