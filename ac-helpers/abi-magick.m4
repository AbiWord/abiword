dnl This is not plugin-specific.
dnl Namespaces are "abi_magickpp_*" and "_abi_magickpp_*"
dnl 
dnl Usage: 
dnl   ABI_MAGICKPP_OPT(<version>,<optional>) where <optional> = "no"|"yes"
dnl 
dnl Defines:
dnl   abi_magickpp_opt=[yes|no|DIR]
dnl   abi_magickpp_config  (if abi_magickpp_opt != no)
dnl   abi_magickpp_version (if abi_magickpp_opt != no)
dnl 

# Check for optional ImageMagick++

AC_DEFUN([ABI_MAGICKPP_OPT], [	
	abi_magickpp_config=""
	abi_magickpp_version=""
	if [ test "x$2" = "xyes" ]; then
		abi_magickpp_opt=check
	else
		abi_magickpp_opt=required
	fi
	AC_ARG_WITH(ImageMagick,[  --with-ImageMagick[=DIR] Use ImageMagick [in DIR] ],[
		if [ test "x$withval" = "xno" ]; then
			if [ test $abi_magickpp_opt = required ]; then
				AC_MSG_ERROR([* * * ImageMagick++ is not optional! * * *])
			fi
			abi_magickpp_opt=no
		elif [ test "x$withval" = "xyes" ]; then
			abi_magickpp_opt=required
			abi_magickpp_dir=""
		else
			abi_magickpp_opt=required
			abi_magickpp_dir="$withval"
		fi
	],[	abi_magickpp_dir=""
	])
	if [ test $abi_magickpp_opt != no ]; then
		if [ test "x$abi_magickpp_dir" = "x" ]; then
			AC_PATH_PROG(abi_magickpp_config,Magick++-config, ,[$PATH])
		else
			AC_PATH_PROG(abi_magickpp_config,Magick++-config, ,[$abi_magickpp_dir/bin:$PATH])
		fi
		if [ test "x$abi_magickpp_config" = "x" ]; then
			if [ test $abi_magickpp_opt = required ]; then
				AC_MSG_ERROR([* * * unable to find Magick++-config in path! * * *])
			fi
			abi_magickpp_opt=no
		fi
	fi
	if [ test $abi_magickpp_opt != no ]; then
	        if [ $abi_magickpp_config --version > /dev/null 2>&1 ]; then
			_abi_magickpp_version="$1"
			_abi_magickpp_major=`echo $_abi_magickpp_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
			_abi_magickpp_minor=`echo $_abi_magickpp_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
			_abi_magickpp_micro=`echo $_abi_magickpp_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

			AC_MSG_CHECKING(for ImageMagick++ >= $_abi_magickpp_major.$_abi_magickpp_minor.$_abi_magickpp_micro)

			abi_magickpp_version=`$abi_magickpp_config --version`
			abi_magickpp_major=`echo $abi_magickpp_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
			abi_magickpp_minor=`echo $abi_magickpp_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
			abi_magickpp_micro=`echo $abi_magickpp_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

			abi_magickpp_version=""
			if [ test $abi_magickpp_major -gt $_abi_magickpp_major ]; then
				abi_magickpp_version="$abi_magickpp_major.$abi_magickpp_minor.$abi_magickpp_micro"
			elif [ test $abi_magickpp_major -eq $_abi_magickpp_major ]; then
				if [ test $abi_magickpp_minor -gt $_abi_magickpp_minor ]; then
					abi_magickpp_version="$abi_magickpp_major.$abi_magickpp_minor.$abi_magickpp_micro"
				elif [ test $abi_magickpp_minor -eq $_abi_magickpp_minor ]; then
					if [ test $abi_magickpp_micro -ge $_abi_magickpp_micro ]; then
						abi_magickpp_version="$abi_magickpp_major.$abi_magickpp_minor.$abi_magickpp_micro"
					fi
				fi
			fi
			if [ test "x$abi_magickpp_version" = "x" ]; then
				if [ test $abi_magickpp_opt = required ]; then
					AC_MSG_ERROR([* * * ImageMagick++ version is incompatible! require at least "1.2.$1" * * *])
				fi
				abi_magickpp_opt=no
				AC_MSG_RESULT(no - $abi_magickpp_major.$abi_magickpp_minor.$abi_magickpp_micro)
			else
				AC_MSG_RESULT(yes - $abi_magickpp_version)
			fi
		else
			AC_MSG_WARN([* * * problem obtaining ImageMagick++ version... * * *])
			if [ test $abi_magickpp_opt = required ]; then
				AC_MSG_ERROR([* * * unable to determine ImageMagick++ version! * * *])
			fi
			abi_magickpp_opt=no
		fi
	fi
	if [ test $abi_magickpp_opt != no ]; then
		if [ test "x$abi_magickpp_dir" = "x" ]; then
			abi_magickpp_opt=yes
		else
			abi_magickpp_opt="$abi_magickpp_dir"
		fi
	fi
])
