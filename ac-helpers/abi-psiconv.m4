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

# Check for Psiconv library.
# Supports:
#  *  psiconv in -lpsiconv
#  *  Bundled psiconv if a directory name argument is passed
#
# Hacked from the abi-xml-parser.m4 code
#
# Usage: 
#  ABI_PSICONV
# or
#  ABI_PSICONV(psiconv-dir)

AC_DEFUN([ABI_PSICONV], [

if test "$ABI_NEED_PSICONV" = "yes"; then

abi_found_psiconv="no"

# check for a shared install

if test "$abi_found_psiconv" = "no"; then
	echo "checking for psiconv"
	AC_CHECK_LIB(psiconv, psiconv_write,
		PSICONV_LIBS="-lpsiconv" abi_found_psiconv="yes"
		)
fi

# check for the header file

if test "$abi_found_psiconv" = "yes"; then
	AC_CHECK_HEADER(psiconv.h, 
	[abi_found_psiconvincs="yes"])
	if test "$abi_found_psiconvincs" = "yes"; then
		PSICONV_CFLAGS=""
		abi_psiconv_message="psiconv in $abi_psiconv_libs"
	else 
		#AC_MSG_WARN([psiconv library found but header file missing])
		abi_found_psiconv="no"
		PSICONV_LIBS=""
	fi
fi


# otherwise, use the sources given as an argument.  [ this means the
# peer dir for abi ]

if test "$abi_found_psiconv" = "no"; then
    if test "x$1" != "x" -a -a "$1"; then
	PSICONV_LIBS="$1/psiconv/.libs/libpsiconv.a"
	PSICONV_CFLAGS="-I$1/"
	AC_MSG_RESULT(using supplied psiconv library)	
	AC_DEFINE(HAVE_PSICONV, 1, [ Define if you have psiconv ])
	abi_psiconv_message="supplied psiconv in $1"
	local_psiconv="true"
	AM_CONDITIONAL(LOCAL_PSICONV, test "$local_psiconv" = "true")
    else
	AC_MSG_ERROR([ psiconv was not found ])
    fi

fi

fi

AC_SUBST(PSICONV_CFLAGS)
AC_SUBST(PSICONV_LIBS)

])
