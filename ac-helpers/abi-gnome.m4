# start: abi/ac-helpers/abi-gnome.m4
# 
# Copyright (C) 2002-2003 AbiSource, Inc
# Copyright (C) 2006 Robert Staudinger <robert.staudinger@gmail.com>
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


AC_DEFUN([ABI_GNOMEUI], [

GNOMEUI_REQUIRED_VERSION='2.0'

gnomeui=false
GNOMEUI_CFLAGS=""
GNOMEUI_LIBS=""

gnomeui_modules="libgnomeui-2.0 >= $GNOMEUI_REQUIRED_VERSION"

AC_ARG_ENABLE(gnomeui,[  --enable-gnomeui    Enable use of GnomeProgram ],
[
	if test "x$enableval" = "xyes"; then
		if test "$PLATFORM" != unix; then
			AC_MSG_ERROR([sorry: --enable-gnomeui supported only on UNIX platforms])
		fi
		PKG_CHECK_EXISTS([$gnomeui_modules], 
		[
			gnomeui=true
			abi_gnomeui_message="as requested"
		], [
			abi_gnomeui_message=">= $GNOMEUI_REQUIRED_VERSION not fulfilled"
		])
	else
		abi_gnomeui_message="as requested"
	fi
], [
	PKG_CHECK_EXISTS([$gnomeui_modules], 
	[
		gnomeui=true
		abi_gnomeui_message="autodetected"
	], [
		abi_gnomeui_message=">= $GNOMEUI_REQUIRED_VERSION not fulfilled"
	])
])

if test "$gnomeui" = true ; then
	PKG_CHECK_MODULES(GNOMEUI,[ $gnomeui_modules ], 
	[
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS $gnomeui_modules"		
	])
	GNOMEUI_CFLAGS="$GNOMEUI_CFLAGS -DHAVE_GNOMEUI=1"
	abi_gnomeui_message="yes ($abi_gnomeui_message)"
else
	abi_gnomeui_message="no ($abi_gnomeui_message)"
fi

#GNOMEUI_CFLAGS="-DGNOMEUI_DISABLE_DEPRECATED $GNOMEUI_CFLAGS"
AC_SUBST(GNOMEUI_CFLAGS)
AC_SUBST(GNOMEUI_LIBS)
	
AM_CONDITIONAL(WITH_GNOMEUI, test "x$gnomeui" = "xtrue")

])



AC_DEFUN([ABI_BONOBO], [

BONOBO_REQUIRED_VERSION='2.0'

bonobo=false
BONOBO_CFLAGS=""
BONOBO_LIBS=""

bonobo_modules="libbonoboui-2.0 >= $BONOBO_REQUIRED_VERSION"

AC_ARG_ENABLE(bonobo,[  --enable-bonobo    Build bonobo widget ],
[
	if test "x$enableval" = "xyes"; then
		if test "$PLATFORM" != unix; then
			AC_MSG_ERROR([sorry: --enable-bonobo supported only on UNIX platforms])
		fi
		PKG_CHECK_EXISTS([$bonobo_modules], 
		[
			bonobo=true
			abi_bonobo_message="as requested"
		], [
			abi_bonobo_message=">= $BONOBO_REQUIRED_VERSION not fulfilled"
		])
	else
		abi_bonobo_message="as requested"
	fi
], [
	PKG_CHECK_EXISTS([$bonobo_modules], 
	[
		bonobo=true
		abi_bonobo_message="autodetected"
	], [
		abi_bonobo_message=">= $BONOBO_REQUIRED_VERSION not fulfilled"
	])
])

if test "$bonobo" = true ; then
	PKG_CHECK_MODULES(BONOBO,[ $bonobo_modules ],
	[
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS $bonobo_modules"
	])
	BONOBO_CFLAGS="$BONOBO_CFLAGS -DHAVE_BONOBO=1"
	abi_bonobo_message="yes ($abi_bonobo_message)"
else
	abi_bonobo_message="no ($abi_bonobo_message)"
fi

#BONOBO_CFLAGS="-DBONOBO_DISABLE_DEPRECATED $BONOBO_CFLAGS"
AC_SUBST(BONOBO_CFLAGS)
AC_SUBST(BONOBO_LIBS)
	
AM_CONDITIONAL(WITH_BONOBO, test "x$bonobo" = "xtrue")

])

# 
# end: abi/ac-helpers/abi-gnome.m4
# 
