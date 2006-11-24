# Copyright (C) 2001 Sam Tobin-Hochstadt
# This file is free software; you may copy and/or distribute it with
# or without modifications, as long as this notice is preserved.
# This software is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.

# The above license applies to THIS FILE ONLY, the abiword code
# itself may be copied and distributed under the terms of the GNU
# GPL, see COPYING for more details

# Check for Wv library.

AC_DEFUN([ABI_WV], [

	# Thank goodness, or fjf, for using pkgconfig now.
	PKG_CHECK_MODULES(WV,[wv-1.0 >= 1.2.0], 
	[
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS wv-1.0 >= 1.2.0"
	])

	AC_SUBST(WV_LIBS)
	AC_SUBST(WV_CFLAGS)

	abi_wv_message="$WV_LIBS"
	
	AC_DEFINE(HAVE_WV, 1, [ Define if you have wv ])
])
