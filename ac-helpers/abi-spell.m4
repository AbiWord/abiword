# start: abi/ac-helpers/abi-spell.m4
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
# Usage: ABI_SPELL

AC_DEFUN([ABI_SPELL],[

SPELL_CFLAGS=""
SPELL_LIBS=""

PKG_CHECK_MODULES(SPELL,[enchant >= 1.2.0])

AM_CONDITIONAL(WITH_ENCHANT,[/bin/true])
AM_CONDITIONAL(WITH_ISPELL,[/bin/false])

AC_SUBST(SPELL_CFLAGS)
AC_SUBST(SPELL_LIBS)

])

# 
# end: abi/ac-helpers/abi-spell.m4
# 
