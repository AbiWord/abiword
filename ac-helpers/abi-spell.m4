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

abi_spell=check

AC_ARG_ENABLE(enchant,[  --disable-enchant    don't use enchant spell-wrapper],[
	if test "x$enableval" = "xno"; then
		abi_spell=disenchanted
	fi
])

if test $abi_spell = check; then
	PKG_CHECK_MODULES(_abi_enchant,[enchant >= 1.1.0],[
		abi_spell=enchant
	],[	abi_spell=check
	])
fi

if test $abi_spell = enchant; then
	SPELL_CFLAGS="$_abi_enchant_CFLAGS -DHAVE_ENCHANT=1"
	SPELL_LIBS="$_abi_enchant_LIBS"
	abi_spell_message="enchant"
else
	ABI_SPELL_CHECK
fi

AM_CONDITIONAL(WITH_ENCHANT,[test $abi_spell = enchant])
AM_CONDITIONAL(WITH_ISPELL, [test $abi_spell = ispell])

])

AC_DEFUN([ABI_SPELL_CHECK],[

abi_spell=ispell
abi_spell_message="ispell"
SPELL_CFLAGS="-DHAVE_ISPELL=1"

AC_SUBST(SPELL_CFLAGS)
AC_SUBST(SPELL_LIBS)

])

# 
# end: abi/ac-helpers/abi-pspell.m4
# 
