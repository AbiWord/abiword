#!/bin/sh

# Building AbiCalc requires that the glib library be
# installed into abi/dist.  The files needed are:
# glib.h			the library's header file
# glibconfig.h		required by glib.h
# gmodule.h			header file for modules
# glib-1.1.lib		the static version of the library
# glib-1.1.dll		the dynamic version of the library
# 
# This script copies the header files into dist/PLATFORM/include and
# the libraries into dist/PLATFORM/lib.
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

# TODO someday we need to be able to do release builds!
OBJDIR=${OS_NAME}_${OS_RELEASE}_${OS_ARCH}_DBG

DISTDIR=./dist/${OBJDIR}

if [ ! -d ./dist ]; then
    mkdir -p ./dist
fi

if [ ! -d ${DISTDIR} ]; then
    mkdir -p ${DISTDIR}
fi

if [ ! -d ${DISTDIR}/lib ]; then
    mkdir -p ${DISTDIR}/bin
fi

if [ ! -d ${DISTDIR}/lib ]; then
    mkdir -p ${DISTDIR}/lib
fi

if [ ! -d ${DISTDIR}/include ]; then
    mkdir -p ${DISTDIR}/include
fi

# the following gyrations can be simplified once autoconf works on NT
if [ $OS_NAME = "WIN32" ]; then
    cp ../glib/glib-1.1.dll	${DISTDIR}/lib/glib-1.1.dll
    cp ../glib/glib-1.1.lib	${DISTDIR}/lib/glib-1.1.lib
    cp ../glib/glib.h ${DISTDIR}/include
    cp ../glib/glibconfig.h ${DISTDIR}/include
    cp ../glib/gmodule/gmodule.h ${DISTDIR}/include
else
    cp ../glib/glib-1.1.lib	${DISTDIR}/lib/glib-1.1.lib
    cp ../glib/glib.h ${DISTDIR}/include
    cp ../glib/gmodule/gmodule.h ${DISTDIR}/include
fi


