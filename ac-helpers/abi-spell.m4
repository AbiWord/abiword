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

AC_ARG_ENABLE(spellcheck,[  --disable-spellcheck    Disable spell-checking capabilities],[
	case "${enableval}" in
	   yes)	do_spell=true ;;
	   no)	do_spell=false ;;
	    *)	AC_MSG_ERROR(bad value ${enableval} for --disable-spell) ;;
	esac
],[
	do_spell=true
])

AM_CONDITIONAL(ENABLE_SPELL, test x$do_spell != xfalse)

if test x$do_spell != xfalse ; then
	PKG_CHECK_MODULES(SPELL,[enchant >= 1.2.0], 
	[
		ABIWORD_REQUIRED_PKGS="$ABIWORD_REQUIRED_PKGS enchant >= 1.2.0"
 	])
	SPELL_CFLAGS="$SPELL_CFLAGS -DENABLE_SPELL"
	AC_DEFINE(WITH_ENCHANT, 1, [ Define if you have Enchant ])
fi

AM_CONDITIONAL(WITH_ENCHANT,[test x$do_spell != xfalse])
AM_CONDITIONAL(WITH_ISPELL,[test /bin/false])

AC_SUBST(SPELL_CFLAGS)
AC_SUBST(SPELL_LIBS)
 
abi_spell_message="enchant"

])

# 
# end: abi/ac-helpers/abi-pspell.m4
# 
