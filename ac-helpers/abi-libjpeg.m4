# Copyright (C) 2001 Sam Tobin-Hochstadt
# Copyright (C) 2001 Hubert Figuiere
# This file is free software; you may copy and/or distribute it with
# or without modifications, as long as this notice is preserved.
# This software is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.

# The above license applies to THIS FILE ONLY, the abiword code
# itself may be copied and distributed under the terms of the GNU
# GPL, see COPYING for more details

# Check for libjpeg
# Supports:
#  *  libjpeg in system library locations
#
# Hacked from the abi-libpng-parser.m4 code
#
# Usage: 
#  ABI_LIBJPEG

AC_DEFUN([ABI_LIBJPEG], [

abi_found_libjpeg="no"

AC_ARG_WITH(libjpeg-prefix, [  --with-libjpeg-prefix=PFX   Prefix where libjpeg is installed (optional)], libjpeg_prefix="$withval", libjpeg_prefix="")

# check for library file using prefix
if test "x$libjpeg_prefix" != "x"; then
    if test -f "$libjpeg_prefix/lib/libjpeg.a"; then
           good_libjpeg_prefix="yes"
       else
           good_libjpeg_prefix="no"
    fi
fi


# check for shared install, with or without prefix
if test "$good_libjpeg_prefix" = "yes"; then
    saved_ldflags="$LDFLAGS"
    LDFLAGS="-L$libjpeg_prefix/lib"
    AC_CHECK_LIB(jpeg, jpeg_start_decompress,
        abi_libjpeg_libs="-L$libjpeg_prefix/lib -ljpeg" abi_found_libjpeg="yes",
	abi_found_libjpeg="no")
    LDFLAGS="$saved_ldflags"
else
    if test "$abi_found_libjpeg" = "no"; then
	echo "checking for libjpeg"
	AC_CHECK_LIB(jpeg, jpeg_start_decompress,
           abi_libjpeg_libs="-ljpeg" abi_found_libjpeg="yes",
           abi_found_libjpeg="no")
    fi
fi

# check for header file
if test "$abi_found_libjpeg" = "yes"; then
    if test "$good_libjpeg_prefix" = "yes"; then
        if test -f "$libjpeg_prefix/include/png.h"; then
            abi_found_libjpegincs="yes"
        else
            abi_found_libjpegincs="no"
        fi
	if test "$abi_found_libjpegincs" = "yes"; then
            LIBJPEG_LIBS="$abi_libjpeg_libs"
            LIBJPEG_CFLAGS="-I$libjpeg_prefix/include -DHAVE_LIBJPEG"
	fi
    else
       AC_CHECK_HEADER(jpeglib.h,abi_found_libjpegincs="yes")
       if test "$abi_found_libjpegincs" = "yes"; then
           LIBJPEG_LIBS="$abi_libjpeg_libs"
           LIBJPEG_CFLAGS="-DHAVE_LIBJPEG"
       fi
    fi   
fi

if test "$abi_found_libjpeg" = "yes" -a "$abi_found_libjpegincs" = "yes"; then
	libjpeg="true"
fi


AC_SUBST(LIBJPEG_CFLAGS)
AC_SUBST(LIBJPEG_LIBS)
AM_CONDITIONAL(WITH_LIBJPEG, test "x$libjpeg" = "xtrue")

])
