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
#### TODO if it is Darwin, we suspect that we have MacOS X, hence we 
#### build MacOS version using Carbon. Change later when we 
#### support Darwin running X and other varieties (like MacOS X
#### using Cocoa). <hfiguiere@teaser.fr>

if test "$OS_NAME" = "Darwin"; then
	OS_NAME=MACOSX
fi

# At this point, we now have the following info:
# OS_NAME = something like 'Linux'
# OS_RELEASE = something like '2.4.1'
# Additionally, there may be info about Cygwin versions.

# huge nasty case statement to actually pick the platform

case "$OS_NAME" in 
	WIN32) PLATFORM="win" ;;
	Linux|AIX|*BSD|IRIX*|HP|OSF1) PLATFORM="unix" ;;
	QNX|procnto) PLATFORM="qnx" ;;
	MACOSX) PLATFORM="mac" ;;
	BeOS) PLATFORM="beos" ;;
esac

AC_SUBST(PLATFORM)

])
