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

dnl Check for pspell

SPELL_CFLAGS=""
SPELL_LIBS=""

ABI_PSPELL_OPT(0.12.0,yes)

if test "$abi_pspell_opt" = yes; then
	if test "$abi_gnu_aspell" = no; then
		abi_spell_message="pspell in system location"
	else
		abi_spell_message="GNU aspell in system location"
	fi
	abi_spell=psyspell
elif test "$abi_pspell_opt" != no; then
	if test "$abi_gnu_aspell" = no; then
		abi_spell_message="pspell in $abi_pspell_opt"
	else
		abi_spell_message="GNU aspell in $abi_pspell_opt"
	fi
	abi_spell=pspell
else
	abi_spell=ispell
fi

if test $abi_spell != ispell; then
	if test "$abi_gnu_aspell" = no; then
		abi_pspell_cflags=""
		abi_pspell_libs="-lpspell -lpspell-modules -lltdl"
	else
		abi_pspell_cflags=""
		abi_pspell_libs="-lpspell -laspell -laspell-common -lltdl"
	fi
	if test $abi_spell = pspell; then
		abi_pspell_cflags="-I$abi_pspell_opt/include"
		abi_pspell_libs="-L$abi_pspell_opt/lib $abi_pspell_libs"

		_abi_cppflags="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $abi_pspell_cflags"
	fi
	AC_CHECK_HEADER(pspell/pspell.h,
	[
	    AC_CHECK_LIB(pspell,new_pspell_config,[
			SPELL_LIBS="$abi_pspell_libs"
			SPELL_CFLAGS="$abi_pspell_cflags -DHAVE_PSPELL=1"
			abi_spell_default=no
			abi_have_new_pspell_config=yes
	    ],[abi_have_new_pspell_config=no],
	    $abi_pspell_libs)
	    if test $abi_have_new_pspell_config = no; then
	    AC_CHECK_LIB(pspell,new_aspell_config,[
			SPELL_LIBS="$abi_pspell_libs"
			SPELL_CFLAGS="$abi_pspell_cflags -DHAVE_PSPELL=1"
			abi_spell_default=no
			abi_have_new_aspell_config=yes
	    ],[abi_have_new_aspell_config=no],
	    $abi_pspell_libs)
	    fi
	    if test $abi_have_new_pspell_config = no; then
	    if test $abi_have_new_aspell_config = no; then
	    AC_MSG_WARN([* * * pspell not found in system location * * *])
	    abi_spell_default=yes
	    fi
	    fi
	],
	[
	    AC_MSG_WARN([* * * pspell not found in system location * * *])
	    abi_spell_default=yes
	])
	if test $abi_spell = pspell; then
		CPPFLAGS="$_abi_cppflags"
	else
		abi_spell=pspell
	fi
	if test $abi_spell_default = yes; then
		abi_spell=ispell
	fi
fi

if test $abi_spell = ispell; then
	abi_spell_message="(ispell)"
	SPELL_CFLAGS="-DHAVE_ISPELL=1"
fi

AC_SUBST(SPELL_CFLAGS)
AC_SUBST(SPELL_LIBS)

AM_CONDITIONAL(WITH_PSPELL,test $abi_spell = pspell)

])

dnl Namespaces are "abi_pspell_*" and "_abi_pspell_*"; also "abi_gnu_aspell"
dnl 
dnl Usage: 
dnl   ABI_PSPELL_OPT(<version>,<optional>[,"no"]) where <optional> = "no"|"yes"
dnl 
dnl Defines:
dnl   abi_pspell_opt=[yes|no|DIR]
dnl   abi_pspell_config  (if abi_pspell_opt != no)
dnl   abi_pspell_version (if abi_pspell_opt != no)
dnl 
dnl   abi_gnu_aspell=[yes|no]

# Check for optional pspell

AC_DEFUN([ABI_PSPELL_OPT], [	
	abi_pspell_config=""
	abi_pspell_version=""
	if [ test "x$2" = "xyes" ]; then
		abi_pspell_opt=check
	else
		abi_pspell_opt=required
	fi
	AC_ARG_ENABLE(pspell,[  --disable-pspell      Use pspell [in DIR] ],[
		if test "x$enableval" = "xno"; then
			if test $abi_pspell_opt = required; then
				AC_MSG_ERROR([* * * pspell is not optional! * * *])
			else
				abi_pspell_opt=no
			fi
		else
			abi_pspell_opt=required
		fi
	])
	AC_ARG_WITH(pspell,[  --with-pspell[=DIR]     Use pspell [in DIR] ],[
		if test $abi_pspell_opt != no; then
			if test "x$withval" = "xno"; then
				if [ test $abi_pspell_opt = required ]; then
					AC_MSG_ERROR([* * * pspell is not optional! * * *])
				fi
				abi_pspell_opt=no
			elif test "x$withval" = "xyes"; then
				abi_pspell_opt=required
				abi_pspell_dir=""
			else
				abi_pspell_opt=required
				abi_pspell_dir="$withval"
			fi
		fi
	],[	abi_pspell_dir=$3
	])
	if [ test $abi_pspell_opt != no ]; then
		if [ test "x$abi_pspell_dir" = "x" ]; then
			AC_PATH_PROG(abi_pspell_config,pspell-config, ,[$PATH])
		else
			AC_PATH_PROG(abi_pspell_config,pspell-config, ,[$abi_pspell_dir/bin:$PATH])
		fi
		if [ test "x$abi_pspell_config" = "x" ]; then
			if [ test $abi_pspell_opt = required ]; then
				AC_MSG_ERROR([* * * unable to find pspell-config in path! * * *])
			fi
			abi_pspell_opt=no
		fi
	fi
	if [ test $abi_pspell_opt != no ]; then
	        if [ $abi_pspell_config --version > /dev/null 2>&1 ]; then
			_abi_pspell_version="$1"
			_abi_pspell_major=`echo $_abi_pspell_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
			_abi_pspell_minor=`echo $_abi_pspell_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
			_abi_pspell_micro=`echo $_abi_pspell_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

			AC_MSG_CHECKING(for pspell >= $_abi_pspell_major.$_abi_pspell_minor.$_abi_pspell_micro)

			abi_pspell_version="0`$abi_pspell_config --version`"
			abi_pspell_major=`echo $abi_pspell_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
			abi_pspell_minor=`echo $abi_pspell_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
			abi_pspell_micro=`echo $abi_pspell_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

			abi_pspell_version=""
			if [ test $abi_pspell_major -gt $_abi_pspell_major ]; then
				abi_pspell_version="$abi_pspell_major.$abi_pspell_minor.$abi_pspell_micro"
			elif [ test $abi_pspell_major -eq $_abi_pspell_major ]; then
				if [ test $abi_pspell_minor -gt $_abi_pspell_minor ]; then
					abi_pspell_version="$abi_pspell_major.$abi_pspell_minor.$abi_pspell_micro"
				elif [ test $abi_pspell_minor -eq $_abi_pspell_minor ]; then
					if [ test $abi_pspell_micro -ge $_abi_pspell_micro ]; then
						abi_pspell_version="$abi_pspell_major.$abi_pspell_minor.$abi_pspell_micro"
					fi
				fi
			fi
			abi_gnu_aspell="no"
			if [ test $abi_pspell_major -gt 0 ]; then
				abi_gnu_aspell="yes"
			elif [ test $abi_pspell_major -eq 0 ]; then
				if [ test $abi_pspell_minor -ge 50 ]; then
					abi_gnu_aspell="yes"
				fi
			fi
			if [ test "x$abi_pspell_version" = "x" ]; then
				if [ test $abi_pspell_opt = required ]; then
					AC_MSG_ERROR([* * * pspell version is incompatible! require at least "1.2.$1" * * *])
				fi
				abi_pspell_opt=no
				AC_MSG_RESULT(no - $abi_pspell_major.$abi_pspell_minor.$abi_pspell_micro)
			else
				abi_real_pspell_version=`echo $abi_pspell_version | sed 's/00/0/'`
				AC_MSG_RESULT(yes - $abi_real_pspell_version)
			fi
		else
			AC_MSG_WARN([* * * problem obtaining pspell version... * * *])
			if [ test $abi_pspell_opt = required ]; then
				AC_MSG_ERROR([* * * unable to determine pspell version! * * *])
			fi
			abi_pspell_opt=no
		fi
	fi
	if [ test $abi_pspell_opt != no ]; then
		if [ test "x$abi_pspell_dir" = "x" ]; then
			abi_pspell_opt=yes
		else
			abi_pspell_opt="$abi_pspell_dir"
		fi
	fi
])
# 
# end: abi/ac-helpers/abi-pspell.m4
# 
