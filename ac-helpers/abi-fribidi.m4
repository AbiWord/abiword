# start: abi/ac-helpers/abi-fribidi.m4
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
# Usage: ABI_FRIBIDI

AC_DEFUN([ABI_FRIBIDI],[

dnl detects fribidi

ABI_FRIBIDI_OPT(0.10.4,no)

FRIBIDI_CFLAGS="`$abi_fribidi_config --cflags` -DBIDI_ENABLED"
FRIBIDI_LIBS="`$abi_fribidi_config --libs`"

AC_SUBST(FRIBIDI_CFLAGS)
AC_SUBST(FRIBIDI_LIBS)

])

dnl Namespaces are "abi_fribidi_*" and "_abi_fribidi_*"
dnl 
dnl Usage: 
dnl   ABI_FRIBIDI_OPT(<version>,<optional>) where <optional> = "no"|"yes"
dnl 
dnl Defines:
dnl   abi_fribidi_opt=[yes|no|DIR]
dnl   abi_fribidi_config  (if abi_fribidi_opt != no)
dnl   abi_fribidi_version (if abi_fribidi_opt != no)
dnl 

# Check for optional fribidi

AC_DEFUN([ABI_FRIBIDI_OPT], [	
	abi_fribidi_config=""
	abi_fribidi_version=""
	if [ test "x$2" = "xyes" ]; then
		abi_fribidi_opt=check
	else
		abi_fribidi_opt=required
	fi
	AC_ARG_WITH(fribidi,[  --with-fribidi[=DIR]     Use fribidi [in DIR] ],[
		if [ test "x$withval" = "xno" ]; then
			if [ test $abi_fribidi_opt = required ]; then
				AC_MSG_ERROR([* * * fribidi is not optional! * * *])
			fi
			abi_fribidi_opt=no
		elif [ test "x$withval" = "xyes" ]; then
			abi_fribidi_opt=required
			abi_fribidi_dir=""
		else
			abi_fribidi_opt=required
			abi_fribidi_dir="$withval"
		fi
	],[	abi_fribidi_dir=""
	])
	if [ test $abi_fribidi_opt != no ]; then
		if [ test "x$abi_fribidi_dir" = "x" ]; then
			AC_PATH_PROG(abi_fribidi_config,fribidi-config, ,[$PATH])
		else
			AC_PATH_PROG(abi_fribidi_config,fribidi-config, ,[$abi_fribidi_dir/bin $PATH])
		fi
		if [ test "x$abi_fribidi_config" = "x" ]; then
			if [ test $abi_fribidi_opt = required ]; then
				AC_MSG_ERROR([* * * unable to find fribidi-config in path! * * *])
			fi
			abi_fribidi_opt=no
		fi
	fi
	if [ test $abi_fribidi_opt != no ]; then
	        if [ $abi_fribidi_config --version > /dev/null 2>&1 ]; then
			_abi_fribidi_version="$1"
			_abi_fribidi_major=`echo $_abi_fribidi_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
			_abi_fribidi_minor=`echo $_abi_fribidi_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
			_abi_fribidi_micro=`echo $_abi_fribidi_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

			AC_MSG_CHECKING(for fribidi >= $_abi_fribidi_major.$_abi_fribidi_minor.$_abi_fribidi_micro)

			abi_fribidi_version=`$abi_fribidi_config --version`
			abi_fribidi_major=`echo $abi_fribidi_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
			abi_fribidi_minor=`echo $abi_fribidi_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
			abi_fribidi_micro=`echo $abi_fribidi_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

			abi_fribidi_version=""
			if [ test $abi_fribidi_major -gt $_abi_fribidi_major ]; then
				abi_fribidi_version="$abi_fribidi_major.$abi_fribidi_minor.$abi_fribidi_micro"
			elif [ test $abi_fribidi_major -eq $_abi_fribidi_major ]; then
				if [ test $abi_fribidi_minor -gt $_abi_fribidi_minor ]; then
					abi_fribidi_version="$abi_fribidi_major.$abi_fribidi_minor.$abi_fribidi_micro"
				elif [ test $abi_fribidi_minor -eq $_abi_fribidi_minor ]; then
					if [ test $abi_fribidi_micro -ge $_abi_fribidi_micro ]; then
						abi_fribidi_version="$abi_fribidi_major.$abi_fribidi_minor.$abi_fribidi_micro"
					fi
				fi
			fi
			if [ test "x$abi_fribidi_version" = "x" ]; then
				if [ test $abi_fribidi_opt = required ]; then
					AC_MSG_ERROR([* * * fribidi version is incompatible! require at least "1.2.$1" * * *])
				fi
				abi_fribidi_opt=no
				AC_MSG_RESULT(no - $abi_fribidi_major.$abi_fribidi_minor.$abi_fribidi_micro)
			else
				AC_MSG_RESULT(yes - $abi_fribidi_version)
			fi
		else
			AC_MSG_WARN([* * * problem obtaining fribidi version... * * *])
			if [ test $abi_fribidi_opt = required ]; then
				AC_MSG_ERROR([* * * unable to determine fribidi version! * * *])
			fi
			abi_fribidi_opt=no
		fi
	fi
	if [ test $abi_fribidi_opt != no ]; then
		if [ test "x$abi_fribidi_dir" = "x" ]; then
			abi_fribidi_opt=yes
		else
			abi_fribidi_opt="$abi_fribidi_dir"
		fi
	fi
])
# 
# end: abi/ac-helpers/abi-fribidi.m4
# 
