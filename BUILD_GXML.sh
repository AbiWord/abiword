#!/bin/sh

# Building AbiCalc requires that the Gnome XML library be
# installed into abi/dist on Windows.  Three files from there are needed:
#	parser.h        parser header file
#	tree.h			tree header file
#	libxml_s.lib    the library itself
# 
# This script copies the header files into dist/PLATFORM/include/gnome-xml and
# the library into dist/PLATFORM/lib.
# The Makefiles for AbiCalc will expect to find them there.
#

OS_NAME=`uname -s | sed "s/\//-/"`
OS_RELEASE=`uname -r | sed "s/\//-/"`
OS_ARCH=`uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ | sed "s/\//-/"`

# uname -s on CygWin returns "CYGWIN32_NT"
if [ $OS_NAME = "CYGWIN32_NT" ]; then
    OS_NAME=WIN32
fi
if [ $OS_NAME = "CYGWIN32_95" ]; then
    OS_NAME=WIN32
fi

if [ $OS_NAME = "WIN32" ]; then
# The win32 makefile does the appropriate copying to the correct abi dirs
    cd ../gnomexml
    make -f makefile.w32 clean
    make -f makefile.w32
    cd ../abi
else
    cd ../js
    ./configure
    ./make
    cd ../abi
fi
