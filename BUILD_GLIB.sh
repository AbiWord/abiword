#!/bin/sh

# Building AbiCalc for Windows requires that the glib library be
# installed into abi/dist. 
# 
# This script compiles glib and calls GET_GLIB.sh to copy the necessary
# header and library files where the Makefiles for AbiCalc will expect 
# to find them.
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

# 
if [ $OS_NAME = "WIN32" ]; then
    cd ../glib
    nmake -f makefile.msc clean
    nmake -f makefile.msc
    cd ../abi
else
    cd ../glib
    ./configure
    ./make
    cd ../abi
fi

sh GET_GLIB.sh
