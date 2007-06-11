# start: abi/ac-helpers/abi-gnome.m4
# 
# Copyright (C) 2006 AbiSource, Inc
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

AC_DEFUN([ABI_GSF], [
	GSF_REQUIRED_VERSION='1.12.0'
	GSF_MODULES="libgsf-1 >= $GSF_REQUIRED_VERSION"

	PKG_CHECK_MODULES(GSF, [$GSF_MODULES], 
	[
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS $GSF_MODULES"
	])

	PKG_CHECK_MODULES(GSF_HTTP, libgsf-1 >= 1.14.4,
	[
		GSF_CFLAGS="$GSF_CFLAGS -DWITH_GSF_INPUT_HTTP=1"
	],
	[
		AC_MSG_WARN([building without HTTP VFS support])
	])

	AC_SUBST(GSF_CFLAGS)
	AC_SUBST(GSF_LIBS)
])


AC_DEFUN([ABI_GNOMEVFS], [

# Hmm, maybe this should be automatically derived from the version of gsf we require
GNOMEVFS_REQUIRED_VERSION='2.2.0'

gnomevfs=false
GNOMEVFS_CFLAGS=""
GNOMEVFS_LIBS=""

gnomevfs_modules="gnome-vfs-2.0 >= $GNOMEVFS_REQUIRED_VERSION libgsf-gnome-1 >= $GSF_REQUIRED_VERSION"

AC_ARG_ENABLE(gnomevfs,[  --enable-gnomevfs    Turn on gnomevfs ],
[
	if test "x$enableval" = "xyes"; then
		if test "$PLATFORM" != unix; then
			AC_MSG_ERROR([sorry: --enable-gnomevfs supported only on UNIX platforms])
		fi
		PKG_CHECK_EXISTS([$gnomevfs_modules], 
		[
			gnomevfs=true
			abi_gnomevfs_message="as requested"
		], [
			abi_gnomevfs_message=">= $GNOMEVFS_REQUIRED_VERSION not fulfilled"
		])
	else
		abi_gnomevfs_message="as requested"
	fi
], [
	PKG_CHECK_EXISTS([$gnomevfs_modules], 
	[
		gnomevfs=true
		abi_gnomevfs_message="autodetected"
	], [
		abi_gnomevfs_message=">= $GNOMEVFS_REQUIRED_VERSION not fulfilled"
	])
])

if test "$gnomevfs" = true ; then
	PKG_CHECK_MODULES(GNOMEVFS,[ $gnomevfs_modules ], 
	[
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS $gnomevfs_modules"
	])
	GNOMEVFS_CFLAGS="$GNOMEVFS_CFLAGS -DWITH_GNOMEVFS=1"
	abi_gnomevfs_message="yes ($abi_gnomevfs_message)"
else
	abi_gnomevfs_message="no ($abi_gnomevfs_message)"
fi

#GNOMEVFS_CFLAGS="-DGNOMEVFS_DISABLE_DEPRECATED $GNOMEVFS_CFLAGS"
AC_SUBST(GNOMEVFS_CFLAGS)
AC_SUBST(GNOMEVFS_LIBS)
	
AM_CONDITIONAL(WITH_GNOMEVFS, test "x$gnomevfs" = "xtrue")

])
