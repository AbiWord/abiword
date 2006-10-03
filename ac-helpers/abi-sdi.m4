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

AC_DEFUN([ABI_SDI], [

sdi=false
SDI_CFLAGS=""

AC_ARG_ENABLE(sdi,[  --enable-sdi    Turn on SDI (single document interface) ],
[
	if test "x$enableval" = "xyes"; then
		if test "$PLATFORM" != unix; then
			AC_MSG_ERROR([sorry: --enable-sdi supported only on UNIX platforms])
		fi
		sdi=true
		abi_sdi_message="as requested"
	else
		abi_sdi_message="as requested"
	fi
], [
	abi_sdi_message="off by default"
])

if test "$sdi" = true ; then
	SDI_CFLAGS="-DHAVE_SDI=1"
	abi_sdi_message="yes ($abi_sdi_message)"
else
	abi_sdi_message="no ($abi_sdi_message)"
fi

AC_SUBST(SDI_CFLAGS)
	
AM_CONDITIONAL(WITH_SDI, test "x$sdi" = "xtrue")

])
