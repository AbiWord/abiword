# start: abi/ac-helpers/abi-c-inline.m4
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
# Usage: ABI_C_INLINE

AC_DEFUN([ABI_C_INLINE],[

if test "$abi_c_inline" = ""; then
	dnl this test is the one used by autoconf in AC_C_INLINE
	dnl 
	for _abi_kw in inline __inline__ __inline; do
		AC_TRY_COMPILE(, [} $_abi_kw foo() {], [abi_c_inline=$_abi_kw; break])
	done
	if test "$abi_c_inline" != "inline"; then
		CFLAGS="-Dinline=$abi_c_inline $CFLAGS"
		AC_SUBST(CFLAGS)
	fi
fi

])
# 
# end: abi/ac-helpers/abi-c-inline.m4
# 
