# start: abi/ac-helpers/abi-types.m4
# 
# Copyright (C) 2003 Mark Gilbert
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
# Usage: ABI_TYPES

AC_DEFUN([ABI_TYPES],[

dnl detects various type attributes (size for example) that may vary across platforms



AC_CHECK_SIZEOF(long int)

ABISIZEOF_LONG_INT="-DABISIZEOF_LONG_INT=$ac_cv_sizeof_long_int"

ABITYPES_CFLAGS="$ABISIZEOF_LONG_INT"



AC_SUBST(ABITYPES_CFLAGS)

])
# 
# end: abi/ac-helpers/abi-types.m4
# 
