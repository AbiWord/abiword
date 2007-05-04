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
# Usage: ABI_GOFFICE

AC_DEFUN([ABI_GOFFICE], [

GOFFICE_VERSION_REQUIRED='0.4.0'
goffice_modules="libgoffice-0.4 >= $GOFFICE_VERSION_REQUIRED"

PKG_CHECK_MODULES(GOFFICE, [ $goffice_modules ], 
[
	HAVE_SYSTEM_GOFFICE=yes
	ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS $goffice_modules"
], [
	HAVE_SYSTEM_GOFFICE=no
])

if test "x$PLATFORM" != "xunix" ; then
	HAVE_SYSTEM_GOFFICE=yes
fi
if test "x$HAVE_SYSTEM_GOFFICE" = "xyes" ; then
	abi_goffice_message="system"
else
	# using builtin subset
	abi_goffice_message="built in"

	GOFFICE_CFLAGS="-I\$(top_srcdir)/goffice-bits"
	GOFFICE_LIBS="\$(top_builddir)/goffice-bits/libgoffice.a"

	# needed for builtin goffice to build
	AC_SUBST(goffice_datadir, '${datadir}/goffice/${VERSION}')
	AC_SUBST(goffice_libdir, '${libdir}/goffice/${VERSION}')
	AC_SUBST(goffice_icondir, '${datadir}/pixmaps/goffice/${VERSION}')
	AC_SUBST(goffice_localedir, '${prefix}/${DATADIRNAME}/locale')

	AC_SUBST(goffice_plugindir, '${goffice_libdir}/plugins')
	AC_SUBST(goffice_gladedir, '${goffice_datadir}/glade')
fi

AM_CONDITIONAL(WITH_SYSTEM_GOFFICE, test "x$HAVE_SYSTEM_GOFFICE" = "xyes")
AC_SUBST(GOFFICE_CFLAGS)
AC_SUBST(GOFFICE_LIBS)
])
# 
# end: abi/ac-helpers/abi-goffice.m4
# 
