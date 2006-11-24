# start: abi/ac-helpers/abi-fribidi.m4
# 
# Copyright (C) 2003 Francis James Franklin
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
# Usage: ABI_FRIBIDI

AC_DEFUN([ABI_FRIBIDI],[

dnl detects fribidi

FRIBIDI_CFLAGS=""
FRIBIDI_LIBS=""

PKG_CHECK_MODULES(FRIBIDI,[fribidi >= 0.10.4],[
	abi_fribidi=yes
	ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS fribidi >= 0.10.4"
],[	echo ""
	echo "fribidi sources can be downloaded from SourceForge"
	echo ""
	echo "	http://freedesktop.org/Software/FriBidi"
	echo ""
	echo "Note: Don't use the fribidi source which is sometimes"
	echo "      included with AbiWord since that version is for"
	echo "      Windows only."
	AC_MSG_ERROR([* * * fribidi >= 0.10.4 is required * * *])
])
FRIBIDI_CFLAGS="$FRIBIDI_CFLAGS"

AC_SUBST(FRIBIDI_CFLAGS)
AC_SUBST(FRIBIDI_LIBS)

])
# 
# end: abi/ac-helpers/abi-fribidi.m4
# 
