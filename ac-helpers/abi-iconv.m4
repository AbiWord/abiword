# start: abi/ac-helpers/abi-iconv.m4
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
# Usage: ABI_DETECT_ICONV

AC_DEFUN([ABI_DETECT_ICONV], [

dnl iconv is simply the funnest
dnl 
dnl 1. Very old platforms don't have it - fair enough.
dnl 2. Old (and, alas, even some new) platforms have it broken.
dnl 3. Some new platforms have it.
dnl 4. Some people have libiconv installed.
dnl 5. Some people want/need to use peer libiconv.

ABI_LIBICONV_DIR=""
AC_ARG_WITH(libiconv,[  --with-libiconv=DIR   use libiconv in DIR],[
	if [ test "$withval" = "no" ]; then
		abi_libiconv=no
        elif [ test "$withval" = "yes" ]; then
		abi_libiconv=yes
        elif [ test "$withval" = "peer" ]; then
		abi_libiconv=peer
	else
		abi_libiconv=sys
		ABI_LIBICONV_DIR="$withval"
        fi
],[	abi_libiconv=check
])

if test $abi_libiconv = peer; then
	abi_iconv=peer
else
	if test $abi_libiconv = sys; then
		_abi_cppflags="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS -I$ABI_LIBICONV_DIR/include"
	fi
# test for existence & nature of iconv.h
	AC_CHECK_HEADER(iconv.h,[
		AC_TRY_COMPILE([
#include <iconv.h>
		],[
#ifndef _LIBICONV_VERSION
        * * /* not (or old?) libiconv */ * *
#endif
#if _LIBICONV_VERSION < 0x0107
        * * /* old stuff */ * *
#endif
		],[	if test $abi_libiconv = no; then
				AC_MSG_ERROR([found libiconv - can I use it?])
			fi
			abi_iconv=sys_libiconv
		],[	if test $abi_libiconv = sys; then
				AC_MSG_ERROR([(usable) libiconv not found])
			fi
			if test $abi_libiconv = yes; then
				AC_MSG_WARN([libiconv not found in system location])
				abi_iconv=peer
			else
				abi_iconv=sys_iconv
			fi
		])
	],[	abi_iconv=peer
	])
	if test $abi_libiconv = sys; then
		CPPFLAGS="$_abi_cppflags"
	fi
fi

LIBICONV_PEERDIR="$abi_rootdir/libiconv"

if test $abi_iconv = peer; then
	AC_MSG_CHECKING(for libiconv in peer directory)
	if test -d $LIBICONV_PEERDIR; then
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([unable to use libiconv - no peer found])
	fi

	abi_iconv_message="peer libiconv"
	ICONV_INCLUDES='-I$(top_builddir)/../libiconv/include'
	ICONV_LIBS='$(top_builddir)/../libiconv/lib/.libs/libiconv.a'

        PEERDIRS="${PEERDIRS} ${LIBICONV_PEERDIR}"
	PEERS="${PEERS} libiconv"

elif test $abi_iconv = sys_libiconv; then
	if test $abi_libiconv = sys; then
		abi_iconv_message="libiconv in -L$ABI_LIBICONV_DIR/lib -liconv"
		ICONV_INCLUDES="-I$ABI_LIBICONV_DIR/include"
		ICONV_LIBS="-L$ABI_LIBICONV_DIR/lib -liconv"
	else
		abi_iconv_message="libiconv in -liconv"
		ICONV_INCLUDES=""
		ICONV_LIBS="-liconv"
	fi

else
	abi_iconv_message="system iconv"
	ICONV_INCLUDES=""
	ICONV_LIBS=""
fi

AC_SUBST(LIBICONV_PEERDIR)

AC_SUBST(ICONV_INCLUDES)
AC_SUBST(ICONV_LIBS)

])
# 
# end: abi/ac-helpers/abi-iconv.m4
# 
