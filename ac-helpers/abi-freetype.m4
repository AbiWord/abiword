# start: abi/ac-helpers/abi-freetype.m4
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

dnl Namespaces are "abi_freetype_*" and "_abi_freetype_*"
dnl 
dnl Usage: 
dnl   ABI_FREETYPE_OPT(<version>,<optional>) where <optional> = "no"|"yes"
dnl 
dnl Defines:
dnl   abi_freetype_opt=[yes|no|DIR]
dnl   abi_freetype_config  (if abi_freetype_opt != no)
dnl   abi_freetype_version (if abi_freetype_opt != no)
dnl 

# Check for optional freetype

AC_DEFUN([ABI_FREETYPE_OPT], [	
	abi_freetype_config=""
	abi_freetype_version=""
	if [ test "x$2" = "xyes" ]; then
		abi_freetype_opt=check
	else
		abi_freetype_opt=required
	fi
	AC_ARG_WITH(freetype,[  --with-freetype[=DIR]     Use freetype [in DIR] ],[
		if [ test "x$withval" = "xno" ]; then
			if [ test $abi_freetype_opt = required ]; then
				AC_MSG_ERROR([* * * freetype is not optional! * * *])
			fi
			abi_freetype_opt=no
		elif [ test "x$withval" = "xyes" ]; then
			abi_freetype_opt=required
			abi_freetype_dir=""
		else
			abi_freetype_opt=required
			abi_freetype_dir="$withval"
		fi
	],[	abi_freetype_dir=""
	])
	if [ test $abi_freetype_opt != no ]; then
		if [ test "x$abi_freetype_dir" = "x" ]; then
			AC_PATH_PROG(abi_freetype_config,freetype-config, ,[$PATH])
		else
			AC_PATH_PROG(abi_freetype_config,freetype-config, ,[$abi_freetype_dir/bin $PATH])
		fi
		if [ test "x$abi_freetype_config" = "x" ]; then
			if [ test $abi_freetype_opt = required ]; then
				AC_MSG_ERROR([* * * unable to find freetype-config in path! * * *])
			fi
			abi_freetype_opt=no
		fi
	fi
	if [ test $abi_freetype_opt != no ]; then
	        if [ $abi_freetype_config --version > /dev/null 2>&1 ]; then
			_abi_freetype_version="$1"
			_abi_freetype_major=`echo $_abi_freetype_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
			_abi_freetype_minor=`echo $_abi_freetype_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
			_abi_freetype_micro=`echo $_abi_freetype_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

			AC_MSG_CHECKING(for freetype >= $_abi_freetype_major.$_abi_freetype_minor.$_abi_freetype_micro)

			abi_freetype_version=`$abi_freetype_config --version`
			abi_freetype_major=`echo $abi_freetype_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
			abi_freetype_minor=`echo $abi_freetype_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
			abi_freetype_micro=`echo $abi_freetype_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

			abi_freetype_version=""
			if [ test $abi_freetype_major -gt $_abi_freetype_major ]; then
				abi_freetype_version="$abi_freetype_major.$abi_freetype_minor.$abi_freetype_micro"
			elif [ test $abi_freetype_major -eq $_abi_freetype_major ]; then
				if [ test $abi_freetype_minor -gt $_abi_freetype_minor ]; then
					abi_freetype_version="$abi_freetype_major.$abi_freetype_minor.$abi_freetype_micro"
				elif [ test $abi_freetype_minor -eq $_abi_freetype_minor ]; then
					if [ test $abi_freetype_micro -ge $_abi_freetype_micro ]; then
						abi_freetype_version="$abi_freetype_major.$abi_freetype_minor.$abi_freetype_micro"
					fi
				fi
			fi
			if [ test "x$abi_freetype_version" = "x" ]; then
				if [ test $abi_freetype_opt = required ]; then
					AC_MSG_ERROR([* * * freetype version is incompatible! require at least "1.2.$1" * * *])
				fi
				abi_freetype_opt=no
				AC_MSG_RESULT(no - $abi_freetype_major.$abi_freetype_minor.$abi_freetype_micro)
			else
				AC_MSG_RESULT(yes - $abi_freetype_version)
			fi
		else
			AC_MSG_WARN([* * * problem obtaining freetype version... * * *])
			if [ test $abi_freetype_opt = required ]; then
				AC_MSG_ERROR([* * * unable to determine freetype version! * * *])
			fi
			abi_freetype_opt=no
		fi
	fi
	if [ test $abi_freetype_opt != no ]; then
		if [ test "x$abi_freetype_dir" = "x" ]; then
			abi_freetype_opt=yes
		else
			abi_freetype_opt="$abi_freetype_dir"
		fi
	fi
])
# 
# end: abi/ac-helpers/abi-freetype.m4
# 
