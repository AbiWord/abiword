# Copyright (C) 2001 Sam Tobin-Hochstadt
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
# Usage: ABI_DETECT_PLATFORM
# The platform is then available in the PLATFORM variable.
# 
# TODO Rewrite this to use $host, which is likely to be more accurate
# and actually maintained.

AC_DEFUN([ABI_DETECT_PLATFORM], [

dnl require that this is called, so we can make use of $GCC to test
dnl whether we're using a gcc-variant or not.
AC_REQUIRE([AC_PROG_CC])

##################################################################
##################################################################
# OS_NAME is the output of uname -s minus any forward slashes
# (so we don't imply another level of depth).  This is to solve
# a problem with BSD/OS.  In fact, it might be good to do this
# to all uname results, so that one doesn't see "sun4/m" as an
# architecture.  The substitutions are taken from the Linux
# kernel Makefile.  The result of this is that even if you have
# a Pentium Pro, you will see your bins in a "...i386" directory.
# This doesn't mean it didn't use your optimizations.

# this makes HP-UX look like "HP" (sed turns "HP-UX" into "HP" with the -.* pattern)
OS_NAME=`uname -s | sed "s/\//-/" | sed "s/_/-/" | sed "s/-.*//g"`
OS_RELEASE=`uname -r | sed "s/\//-/" | sed "s/[() ].*//g"`

##################################################################
##################################################################
#### Cygnus keeps changing the value that uname returns between
#### different versions of the package and between different
#### versions of Windows.  Here we fold them all in into one symbol.

if test "$OS_NAME" = "CYGWIN32"; then
	OS_NAME="WIN32"
fi

if test "$OS_NAME" = "CYGWIN"; then
	OS_NAME="WIN32"
fi

if test "$OS_NAME" = "WIN32"; then
	CYGWIN_MAJOR_VERSION=`echo $(OS_RELEASE) | cut -d . -f 1`
	CYGWIN_MINOR_VERSION=`echo $(OS_RELEASE) | cut -d . -f 2`
	CYGWIN_REVISION=`echo $(OS_RELEASE) | cut -d . -f 3`
fi

##################################################################
##################################################################
#### MINGW configuration

if test "$OS_NAME" = "MINGW32"; then
	OS_NAME="WIN32"
fi

##################################################################
##################################################################
#### Mac OS X / darwin configuration
# 
# Whether to use Darwin's @executable_path dynamic linker feature
# 
EPATH_WV_BUILD_FLAGS=""
AC_ARG_WITH(epath,[  --without-epath      hardcode relative install path in libwv (MacOS X only)],[
	if test "x$withval" = "xno"; then
		abi_epath=no
	elif test "$OS_NAME" = "Darwin"; then
		abi_epath=yes
		EPATH_WV_BUILD_FLAGS="EPATH_DYLIB=1"
	else
		AC_MSG_ERROR([* * * --with-epath is a MacOS X option * * *])
	fi
],[	if test "$OS_NAME" = "Darwin"; then
		abi_epath=yes
		EPATH_WV_BUILD_FLAGS="EPATH_DYLIB=1"
	else
		abi_epath=no
	fi
])
AM_CONDITIONAL(ABI_EPATH_DYLIB,[test $abi_epath = yes])
AC_SUBST(EPATH_WV_BUILD_FLAGS)
# 
# convenience option for building with fink installed...
# 
AC_ARG_WITH(fink,[  --with-fink          add /sw/... to CPP/LDFLAGS (Mac OS X)],[
	if test "x$withval" != "xno"; then
		CPPFLAGS="$CPPFLAGS -I/sw/include"
		LDFLAGS="$LDFLAGS -L/sw/lib"
		PKG_CONFIG_PATH=/sw/lib/pkgconfig:$PKG_CONFIG_PATH
		export PKG_CONFIG_PATH
	fi
])

# 
# convenience option for building with darwin ports installed...
# 
AC_ARG_WITH(darwinports,[  --with-darwinports          add /opt/local/... to CPP/LDFLAGS (Mac OS X)],[
	if test "x$withval" != "xno"; then
		CPPFLAGS="$CPPFLAGS -I/opt/local/include"
		LDFLAGS="$LDFLAGS -L/opt/local/lib"
		PKG_CONFIG_PATH=/opt/local/lib/pkgconfig:$PKG_CONFIG_PATH
		export PKG_CONFIG_PATH
	fi
])

# convenience option for building with the abiportssdk...
# 
AC_ARG_WITH(abiportssdk,[  --with-abiportssdk=SDK_ROOT     add /opt/abi/... to CPP/LDFLAGS and SDK usage (Mac OS X only)],[
        if test "x$withval" != "xno"; then
		if test "x$withval" = xyes ; then
			_sdk_root=/Developer/SDKs
		else
			_sdk_root=$withval
		fi
		dnl SDK has to be set very early. It is Mac specific.
		CPPFLAGS="-isysroot $_sdk_root/AbiMacOSX.sdk $CPPFLAGS"
		LDFLAGS="-isysroot $_sdk_root/AbiMacOSX.sdk -Wl,-syslibroot,$_sdk_root/AbiMacOSX.sdk $LDFLAGS"

                CPPFLAGS="$CPPFLAGS -I/opt/abi/include"
                LDFLAGS="$LDFLAGS -L/opt/abi/lib"
		PKG_CONFIG_PATH=$_sdk_root/AbiMacOSX.sdk/opt/abi/lib/pkgconfig:$PKG_CONFIG_PATH
		export PKG_CONFIG_PATH
		ABI_MACSDK_ROOT=$_sdk_root
		AC_SUBST(ABI_MACSDK_ROOT)
        fi
])

# 
# 1. Whether to consider using Cocoa API:
# 
AC_ARG_ENABLE(Cocoa,[  --disable-cocoa    don't use Cocoa API  (MacOSX builds only)],[
	if [ test "x$enableval" = "xno" ]; then
		abi_gui_cocoa=no
	else
		abi_gui_cocoa=yes
	fi
],abi_gui_cocoa=yes)
# 
# 2. Whether to consider using Carbon API:
# 
# I scraped Carbon stuff (Hub)
# 
# 3. Default to Cocoa, then Carbon, then GTK
#    (Values are equivalent to PLATFORM setting)
# 
if [ test $abi_gui_cocoa = yes ]; then
	abi_gui=cocoa
else
	abi_gui=unix
fi
# 
# 4. For Cocoa or Carbon, recognize Darwin as MACOSX; otherwise, recognize as FreeBSD
# 
if test "$OS_NAME" = "Darwin"; then
	if [ test $abi_gui = unix ]; then
		OS_NAME=FreeBSD
	else
		OS_NAME=MACOSX
	fi
fi

# At this point, we now have the following info:
# OS_NAME = something like 'Linux'
# OS_RELEASE = something like '2.6.1'
# Additionally, there may be info about Cygwin versions.

dnl why not call AC_CANONICAL_HOST or AC_CANONICAL_SYSTEM and
dnl use those values instead?
case "$OS_NAME" in 
	MACOSX)
		# the default for MacOS X is to no enable warnings with
		# the Apple-derived gcc.
		WARNING_CFLAGS=""
		;;
	*BSD|DragonFly)
		WARNING_CFLAGS="-Wall -pedantic -D_BSD_SOURCE -pipe" #-ansi 
		;;
	IRIX*)
		case "$GCC" in
			yes)
				WARNING_CFLAGS="-Wall -pedantic -D_POSIX_SOURCE -pipe" #-ansi
			;;
			no)
				# the default with the IRIX compiler is for warnings to be
				# on.  You can turn individual warnings off using the
				# `-woff #' directive to the MIPSpro compiler.
				WARNING_CFLAGS=""
			;;
		esac
		;;
	SunOS)
		case "$GCC" in
			yes)
				# why -D_BSD_SOURCE?
				WARNING_CFLAGS="-Wall -pedantic -D_BSD_SOURCE -pipe" #-ansi
				WARNING_CFLAGS="$WARNING_CFLAGS -D__EXTENSIONS__ -DSCANDIR_MISSING -DSunOS"
			;;
			no)
				# the default with the SparcWorks 5.x and Forte 6.x and
				# later is for all warnings to be on.  See the
				# -erroff and -errtags section of the cc man page.
				WARNING_CFLAGS=""
			;;
		esac
		;;
	OSF1)
		case "$GCC" in
			yes)
				WARNING_CFLAGS="-Wall -pedantic -D_POSIX_SOURCE -D_BSD_SOURCE -D_OSF_SOURCE -D_XOPEN_SOURCE_EXTENDED -DAES_SOURCE" #-ansi
			;;
			no)
				# the DEC/Compaq compiler has a plethora of options related
				# to warnings.  Enabling `-portable' and then turning off
				# specific, annoying warnings may be the easiest way to go.
				# some versions of the compiler under 4.x might not have
				# supported `-portable', so leave WARNING_CFLAGS empty in
				# that case
				case "$OS_RELEASE" in
					V5*)
						no_msg='hexoctunsign,switchlong,valuepres'
						WARNING_CFLAGS="-portable -msg_disable $no_msg"
						no_msg=''
					;;
					*) WARNING_CFLAGS=""
					;;
				esac
			;;
		esac
		;;
	QNX|procnto)
		WARNING_CLFAGS="-w9 -D_POSIX_SOURCE" #-ansi
		LDFLAGS="-lph -lphrender -lAp"
		;;
	*)
		case "$GCC" in
			yes)
				WARNING_CFLAGS="-Wall -pedantic -D_POSIX_SOURCE" #-ansi
				WARNING_CFLAGS="$WARNING_CFLAGS -D_BSD_SOURCE -pipe"
			;;
			no)
				WARNING_CFLAGS=""
			;;
		esac
		;;
esac

# huge nasty case statement to actually pick the platform

case "$OS_NAME" in 
	WIN32)
		PLATFORM="win"
		;;
	Linux|AIX|*BSD|IRIX*|HP|OSF1|SunOS|DragonFly)
		PLATFORM="unix"
		;;
	QNX|procnto)
		PLATFORM="qnx"
		;;
	MACOSX)
		PLATFORM=$abi_gui
		;;
	*)
	       PLATFORM="unix"
		;;
esac

if test "$PLATFORM" = "cocoa"; then
	BE_PLATFORM="unix"
else
	BE_PLATFORM="$PLATFORM"
fi

AC_SUBST(PLATFORM)
AC_SUBST(BE_PLATFORM)
AC_SUBST(OS_NAME)
AC_SUBST(WARNING_CFLAGS)

])
