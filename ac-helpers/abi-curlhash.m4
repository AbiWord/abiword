# start: abi/ac-helpers/abi-curlhash.m4
# 
# Copyright (C) 2002 Francis James Franklin
# Copyright (C) 2002 AbiSource, Inc
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
# Usage: ABI_CURL
# Usage: ABI_CURLHASH (calls ABI_CURL)

AC_DEFUN([ABI_CURL],[

dnl detects curl
dnl sets abi_curl to yes (curl found) or no (curl not found)
dnl sets:
dnl  LIBCURL_CFLAGS
dnl  LIBCURL_LIBS

ABI_CURL_DIR=""
AC_ARG_WITH(curl,[  --with-curl[=DIR]  Use curl [in DIR]],[
	if test "x$withval" = "xno"; then
		abi_curl=no
	elif test "x$withval" = "xyes"; then
		abi_curl=yes
	else
		abi_curl=yes
		ABI_CURL_DIR="$withval"
	fi
],[	abi_curl=check
])

if test $abi_curl != no; then
	if test "x$ABI_CURL_DIR" = "x"; then
		AC_PATH_PROG(CURL_CONFIG,curl-config, ,[$PATH])
	else
		AC_PATH_PROG(CURL_CONFIG,curl-config, ,[$ABI_CURL_DIR/bin:$PATH])
	fi
	if test "x$CURL_CONFIG" = "x"; then
		if test $abi_curl = yes; then
			AC_MSG_ERROR([unable to find curl-config in path! http://curl.haxx.se/])
		fi
		abi_curl=no
	else
		abi_curl=yes
	fi
fi

if test $abi_curl != no; then
	LIBCURL_CFLAGS="`$CURL_CONFIG --cflags` -DHAVE_CURL=1"
	LIBCURL_LIBS="`$CURL_CONFIG --libs`"
else
	LIBCURL_CFLAGS=""
	LIBCURL_LIBS=""
fi

AC_SUBST(LIBCURL_CFLAGS)
AC_SUBST(LIBCURL_LIBS)

])

AC_DEFUN([ABI_CURLHASH],[

dnl option to use curl to download abispell dictionary hash files
dnl sets CURLHASH_CFLAGS

AC_ARG_ENABLE(curlhash,[  --enable-curlhash  Enable downloading of abispell with curl],[
	if test "x$withval" = "xno"; then
		abi_curlhash=no
	else
		abi_curlhash=yes
	fi
],[	abi_curlhash=no
])

if test $abi_curlhash = yes; then
	if test "x$abi_curl" != "xyes"; then
		ABI_CURL
	fi
	if test $abi_curl = no; then
		AC_MSG_ERROR([curl is required for curlhash! http://curl.haxx.se/])
	fi
fi

if test $abi_curlhash = yes; then
	CURLHASH_CFLAGS="-DHAVE_CURLHASH=1"
else
	CURLHASH_CFLAGS=""
fi

AC_SUBST(CURLHASH_CFLAGS)

])
