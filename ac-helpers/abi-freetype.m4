# start: abi/ac-helpers/abi-freetype.m4
# 
# Copyright (C) 2006 Marc Maurer <uwog@uwog.net>
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
# Usage: ABI_FREETYPE

AC_DEFUN([ABI_FREETYPE],[

dnl detects freetype2

FREETYPE_CFLAGS=""
FREETYPE_LIBS=""
freetype_modules='freetype2 >= 2.1.0'

PKG_CHECK_MODULES(FREETYPE,[$freetype_modules],[
	abi_freetype=yes
	ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS $freetype_modules"
],[	AC_MSG_ERROR([Error: $freetype_modules required])
])

AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

])
# 
# end: abi/ac-helpers/abi-freetype.m4
# 
