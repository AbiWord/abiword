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
#
# Usage: 

AC_DEFUN([ABI_GSF], [
	GSF_REQUIRED_VERSION=1.12.0
	GSF_MODULES="libgsf-1 >= $GSF_REQUIRED_VERSION"

	if test "x$gnome" = "xtrue"; then
		GSF_MODULES="$GSF_MODULES libgsf-gnome-1 >= $GSF_REQUIRED_VERSION"
	fi

	PKG_CHECK_MODULES(GSF,[$GSF_MODULES])

	AC_SUBST(GSF_CFLAGS)
	AC_SUBST(GSF_LIBS)
])
