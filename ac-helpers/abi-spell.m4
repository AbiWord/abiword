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
# This file detects which of the AbiWord platforms we are currently
# building on.  The detection logic in question is mostly by Jeff
# Hostetler, and is taken from the original AbiWord build system.  
#
# Usage: ABI_SPELL

AC_DEFUN([ABI_SPELL],[

dnl Check for pspell

abi_spell=ispell
abi_found_pspell=no

ABI_PSPELL_DIR=""
AC_ARG_WITH(pspell,[  --with-pspell=DIR     use pspell in DIR (default: use ispell)],[
	if [ test "$withval" = "yes" ]; then
		abi_spell=pspell
	elif [ test "$withval" != "no" ]; then
		abi_spell=psyspell
		ABI_PSPELL_DIR="$withval"
        fi
])

if test $abi_spell != ispell; then
	if test $abi_spell = psyspell; then
		_abi_cppflags="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS -I$ABI_PSPELL_DIR/include"
		abi_pspell_libs="-L$ABI_PSPELL_DIR/lib -lpspell -lpspell-modules -lltdl"
	else
		abi_pspell_libs="-lpspell -lpspell-modules -lltdl"
	fi
	AC_CHECK_HEADER(pspell/pspell.h,[
		AC_CHECK_LIB(pspell,new_pspell_config,[
			SPELL_LIBS="$abi_pspell_libs"
		],[	AC_MSG_ERROR([* * * pspell not found in system location * * *])
		],$abi_pspell_libs)
	],[	AC_MSG_ERROR([* * * pspell not found in system location * * *])
	])
	if test $abi_spell = psyspell; then
		CPPFLAGS="$_abi_cppflags"
	fi
fi

if test $abi_spell = ispell; then
	abi_spell_message="(ispell)"
	SPELL_CFLAGS="-DHAVE_ISPELL=1"
	SPELL_LIBS=""
else
	if test $abi_spell = psyspell; then
		abi_spell=pspell
		abi_spell_message="pspell in -L$ABI_PSPELL_DIR/lib $abi_pspell_libs"
		SPELL_CFLAGS="-I$ABI_PSPELL_DIR/include -DHAVE_PSPELL=1"
	else
		abi_spell_message="pspell in $abi_pspell_libs"
		SPELL_CFLAGS="-DHAVE_PSPELL=1"
	fi
fi

AC_SUBST(SPELL_CFLAGS)
AC_SUBST(SPELL_LIBS)

AM_CONDITIONAL(WITH_PSPELL,test $abi_spell = pspell)

])
# 
# end: abi/ac-helpers/abi-pspell.m4
# 
