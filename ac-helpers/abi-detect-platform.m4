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
#### Mac OS X / darwin configuration
# 
# Whether to use Darwin's @executable_path dynamic linker feature
# 
EPATH_WV_BUILD_FLAGS=""
EPATH_WV_INSTALL_IN_=""
AC_ARG_WITH(epath,[  --with-epath         hardcode relative install path in libwv (MacOS X only)],[
	if test "x$withval" = "xno"; then
		abi_epath=no
	elif test "$OS_NAME" = "Darwin"; then
		abi_epath=yes
		EPATH_WV_BUILD_FLAGS="EPATH_DYLIB=1"
		EPATH_WV_INSTALL_IN_="EPATH_DYLIB=1 installdylib libdir="
	else
		AC_MSG_ERROR([* * * --with-epath is a MacOS X option * * *])
	fi
],[	abi_epath=no
])
AM_CONDITIONAL(ABI_EPATH_DYLIB,[test $abi_epath = yes])
AC_SUBST(EPATH_WV_BUILD_FLAGS)
AC_SUBST(EPATH_WV_INSTALL_IN_)
# 
# 1. Whether to consider using Cocoa API:
# 
AC_ARG_ENABLE(Cocoa,[  --disable-Cocoa    don't use Cocoa API  (MacOSX builds only)],[
	if [ test "x$enableval" = "xno" ]; then
		abi_gui_cocoa=no
	else
		abi_gui_cocoa=yes
	fi
],abi_gui_cocoa=yes)
# 
# 2. Whether to consider using Carbon API:
# 
AC_ARG_ENABLE(Carbon,[  --disable-Carbon   don't use Carbon API (MacOSX builds only)],[
	if [ test "x$enableval" = "xno" ]; then
		abi_gui_carbon=no
	else
		abi_gui_carbon=yes
	fi
],abi_gui_carbon=yes)
# 
# 3. Default to Cocoa, then Carbon, then GTK
#    (Values are equivalent to PLATFORM setting)
# 
if [ test $abi_gui_cocoa = yes ]; then
	abi_gui=cocoa
else
	if [ test $abi_gui_carbon = yes ]; then
		abi_gui=mac
	else
		abi_gui=unix
	fi
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
# OS_RELEASE = something like '2.4.1'
# Additionally, there may be info about Cygwin versions.

dnl TODO It would be nice to eventually add -Werror
dnl FIXME these flags are gcc-specific, and need to be fixed
dnl actually MacOS X build does not want some of them because of 
dnl precompiled Mac headers.... with Apple's GCC.
dnl 
case "$OS_NAME" in 
	MACOSX) WARNING_CFLAGS="" ;;
	*BSD)   WARNING_CFLAGS="-Wall -pedantic -ansi -D_BSD_SOURCE -pipe" ;;
	SunOS)  WARNING_CFLAGS="-Wall -pedantic -ansi -D_BSD_SOURCE -pipe -D__EXTENSIONS__ -DSCANDIR_MISSING -DSunOS";;
	OSF1)   WARNING_CFLAGS="-Wall -pedantic -ansi -D_POSIX_SOURCE -D_BSD_SOURCE -D_OSF_SOURCE -D_XOPEN_SOURCE_EXTENDED -DAES_SOURCE" ;;
	*)      WARNING_CFLAGS="-Wall -pedantic -ansi -D_POSIX_SOURCE -D_BSD_SOURCE -pipe" ;;
esac

# huge nasty case statement to actually pick the platform

case "$OS_NAME" in 
	WIN32) PLATFORM="win" ;;
	Linux|AIX|*BSD|IRIX*|HP|OSF1|SunOS) PLATFORM="unix" ;;
	QNX|procnto) PLATFORM="qnx" ;;
	MACOSX) PLATFORM=$abi_gui ;;
	BeOS) PLATFORM="beos" ;;
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
