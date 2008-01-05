# start: abi/ac-helpers/abi-popt.m4
# 
# Copyright (C) 2002 Patrick Lam
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
# Usage: ABI_POPT

AC_DEFUN([ABI_POPT],[

dnl Check for popt
dnl Supports:
dnl  *  popt in system library locations
dnl  *  Bundled popt if a directory name argument is passed

abi_found_popt="no"

ABI_LIBPOPT_DIR=""
AC_ARG_WITH(popt,[  --with-popt=DIR     use popt in DIR],[
	if [ test "$withval" = "no" ]; then
		AC_MSG_ERROR([* * * popt is required by AbiWord * * *])
        elif [ test "$withval" = "yes" ]; then
		abi_libpopt=check
        elif [ test "$withval" = "peer" ]; then
		abi_libpopt=peer
	else
		abi_libpopt=sys
		ABI_LIBPOPT_DIR="$withval"
        fi
],[	abi_libpopt=check
])

if test $abi_libpopt = peer; then
	abi_popt=peer
else
	if test $abi_libpopt = sys; then
		_abi_cppflags="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS -I$ABI_LIBPOPT_DIR/include"
	fi
	AC_CHECK_HEADER(popt.h,[
		abi_popt=sys
	],[	if test $abi_libpopt = sys; then
			AC_MSG_ERROR([* * * libpopt not found in system location * * *])
		fi
		abi_popt=peer
	])
	if test $abi_libpopt = sys; then
		CPPFLAGS="$_abi_cppflags"
	fi
fi

LIBPOPT_PEERDIR="$abi_rootdir/popt"

if test $abi_popt = peer; then
	AC_MSG_CHECKING(for popt in peer directory)
	if test -d $LIBPOPT_PEERDIR; then
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([unable to use popt - no peer found])
	fi

	abi_libpopt_message="peer popt"
	LIBPOPT_CFLAGS='-I$(top_srcdir)/../popt'
	LIBPOPT_LIBS='$(top_builddir)/../popt/.libs/libpopt.a'

	PEERDIRS="${PEERDIRS} ${LIBPOPT_PEERDIR}"
	PEERS="${PEERS} popt"
else
	if test $abi_libpopt = sys; then
		abi_libpopt_message="libpopt in -L$ABI_LIBPOPT_DIR/lib -lpopt"
		LIBPOPT_CFLAGS="-I$ABI_LIBPOPT_DIR/include"
		LIBPOPT_LIBS="-L$ABI_LIBPOPT_DIR/lib -lpopt"
	else
		abi_libpopt_message="libpopt in -lpopt"
		LIBPOPT_CFLAGS=""
		LIBPOPT_LIBS="-lpopt"
	fi
fi

AC_SUBST(LIBPOPT_PEERDIR)

AC_SUBST(LIBPOPT_CFLAGS)
AC_SUBST(LIBPOPT_LIBS)

])
# 
# end: abi/ac-helpers/abi-popt.m4
# 
