#! gmake

## AbiSource Program Utilities
## Copyright (C) 2002 AbiSource, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

##############################################################################
## Mingw platform defines
##############################################################################

# *** WARNING: THESE DEFS WIRE WIN_FE, NO CHOICE, AND ASSUME A NATIVE WIN32 RUNTIME ENVIRONMENT.  ***

# Here for syntactic purposes.
##
PLATFORM_FLAGS	=
PORT_FLAGS		=
OS_CFLAGS               = -mwindows -DDM_SPECVERSION=0x0320
# Earnie Boyd has added DM_SPECVERSION macro (of this value) to MinGW
# CVS tree on 04/14/03 so this is only for versions 2.0.0-3 and earlier
##

# mingw doesn't like -ansi in compiling wv
ABI_OPT_PACIFY_COMPILER=1
# There were changes in wv, not sure we need this any more
# *** We do not support ancient mingw versions of the gcc2 kind ***

ifndef $(OS_ARCH)
OS_ARCH		:= $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc32/ -e s/sparc32/sparc/ -e s/arm.*/arm/ -e s/sa110/arm/ | sed "s/\//-/")
endif
ifndef $(OS_REALARCH)
OS_REALARCH	:= $(shell uname -m)
endif

ALPHA_ARCH_FLAGS 	=
IA64_ARCH_FLAGS		=

# Define tools
CC		= gcc
CCC		= g++
RANLIB	= ranlib
AR		= ar cr $@
RC		= windres $(RCFLAGS) 

# Suffixes
OBJ_SUFFIX	= o
LIB_SUFFIX	= a
DLL_SUFFIX	= so # We dont use actual dlls, for disting anyway.  We could though, but given the windows using church secretary, I think current system is safer.
EXE_SUFFIX = .exe

OBJ_DIR_SFX	=
DEFINES		=
OPTIMIZER	=

ifdef CXXFLAGS
    OPTIMIZER		+= $(CXXFLAGS)
endif
    
ifdef ABI_OPT_PROF
    ifeq ($(ABI_OPT_PROF),1)
    OPTIMIZER   	+= -pg
    OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)PRF_
    ABI_OPT_OPTIMIZE= 1
    ABI_OPT_DEBUG	= 0
    ABI_OPTIONS	+= Profile:On
    endif
    ifeq ($(ABI_OPT_PROF),2)
    OPTIMIZER   	+= -pg -g -fprofile-arcs -ftest-coverage
    OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)PRF_
    ABI_OPT_OPTIMIZE= 1
    ABI_OPT_DEBUG	= 0
    ABI_OPTIONS	+= Profile:On
    endif
	# Level 3 is deprecated in most areas, for the systems used by the people who'd use it.
endif

#Use ABI_OPT_OPTIMIZE=0 to disable optimizations
ifdef ABI_OPT_TINY
	ifeq ($(ABI_OPT_TINY),1)
	    OPTIMIZER	+= -Os -fno-default-inline -fno-inline
	    else
	    ifndef CXXFLAGS
	#	No need to make more trouble for ourselves than necessary
	    OPTIMIZER	+= 
	    endif
	endif	
else
	ifndef ABI_OPT_OPTIMIZE
		OPTIMIZER	+= -O1
	else
		ifeq ($(ABI_OPT_OPTIMIZE),1)
		#	No need to make more trouble for ourselves than necessary
		OPTIMIZER	+= -O2 x
		OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)OPT_
		ABI_OPTIONS	+= Optimize:On
		ABI_OPT_DEBUG	= 0
		    ifeq ($(ABI_OPT_EXCLUSIVE_OPT),1)
		    OPTIMIZER := $(OPTIMIZER) -march=$(OS_REALARCH)
		    else
		#	As soon as 3.4+ is current and not candidate, gotta use mtune
		    OPTIMIZER := $(OPTIMIZER) -mtune=$(OS_REALARCH)
		    endif
		else
		    
		endif
	endif	
endif

ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER	= -g
DEFINES		= -DDEBUG
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)DBG_
endif
ifeq ($(ABI_OPT_DEBUG),2)
OPTIMIZER	= -g3
DEFINES		= -DDEBUG -DUT_DEBUG -DFMT_TEST -DUT_TEST -DPT_TEST -UNDEBUG
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)DBG_
endif

ifdef CXXFLAGS
    OPTIMIZER		+= $(CXXFLAGS)
endif

OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)OBJ

ifneq ($(ABI_OPT_PACIFY_COMPILER),1)
WARNFLAGS	+= -Wall -ansi -pedantic
else
WARNFLAGS	+= -Wall
endif

# Includes
OS_INCLUDES		=
ifeq ($(ABI_REQUIRE_PEER_ICONV),1)
OS_INCLUDES		+= -I$(ABI_ROOT)/../libiconv/include
endif



# Architecture-specific flags
ifeq ($(OS_ARCH), i386)
PLATFORM_FLAGS		+= $(i386_ARCH_FLAGS)
OS_ENDIAN		= LittleEndian32
endif

ifeq ($(OS_ARCH), alpha)
PLATFORM_FLAGS		+= $(ALPHA_ARCH_FLAGS)
OS_ENDIAN		= LittleEndian32
endif

ifeq ($(OS_ARCH), ia64)
PLATFORM_FLAGS		+= $(IA64_ARCH_FLAGS)
OS_ENDIAN		= LittleEndian32
endif

LIBXML_CONFIG	= pkg-config libxml-2.0   # yeah, so, we don't use this yet.  may or may not before 2.0

# Which links can this platform create.  Define one not both of these options.
UNIX_CAN_BUILD_DYNAMIC=0
UNIX_CAN_BUILD_STATIC=1 
# I'm still not totally decided really...I'll need to experiment some more.

# No longer hardcoding for peer iconv - 2 Jun 2006
# ABI_REQUIRE_PEER_ICONV = 1
ABI_REQUIRE_PEER_ICONV = 0

# Currently hard code expat to disable for Win32
ABI_OPT_PEER_EXPAT?=0
ABI_OPT_MSXML?=0

# If user wants MSXML, explicitly disable expat
ifeq ($(ABI_OPT_MSXML),1)
  ABI_OPT_PEER_EXPAT=0
endif 

## add wv's mini glib to include list
#ABI_OTH_INCS+=	/../../wv/glib-wv

#ifneq ($(shell which pkg-config), )
	ABI_GSF_LIB += $(shell pkg-config --libs --silence-errors libgsf-1)
	ABI_GSF_INC += $(shell pkg-config --cflags --silence-errors libgsf-1)
	ABI_GLIB_LIB += $(shell pkg-config --libs --silence-errors glib-2.0 gthread-2.0)
	ABI_GLIB_INC += $(shell pkg-config --cflags --silence-errors glib-2.0 gthread-2.0)
ifneq ($(HAVE_WV_PEER),1)
	ABI_WV_LIB += $(shell pkg-config --libs --silence-errors wv-1.0)
	ABI_WV_INC += $(shell pkg-config --cflags --silence-errors wv-1.0)
endif
#else
#	error 
#endif

# TODO: This is true on about 1/10 platforms.  Why not use pkgconfig?
ifeq ($(ABI_OPT_PANGO),1)
OS_INCLUDES += -I/usr/local/include/pango-1.0 -I/usr/local/include/freetype2 $(ABI_GLIB_INC)
endif

# libgsf
OS_LIBS += $(ABI_GSF_LIB)
ABI_LIBS += libgsf-1
OS_INCLUDES += $(ABI_GSF_INC)

# glib
# ABI_OTH_INCS += $(GLIB_INC)
OS_LIBS += $(ABI_GLIB_LIB)
ABI_LIBS += glib-2.0
OS_INCLUDES += $(ABI_GLIB_INC)

# wv
ifneq ($(HAVE_WV_PEER),1)
OS_LIBS += $(ABI_WV_LIB)
#ABI_LIBS += wv-1.0
OS_INCLUDES += $(ABI_WV_INC)
endif

# zlib
ABI_ZLIB_ROOT = $(ABI_ROOT)/../libs/zlib
OS_LIBS += -L$(ABI_ZLIB_ROOT)/lib
ABI_ZLIB_INC = $(ABI_ZLIB_ROOT)/include
ABI_ZLIB_LIB = -lzdll
ABI_LIBS += zdll 


# so <fribidi.h> works
OS_INCLUDES += -I$(ABI_ROOT)/../fribidi	
# Do this instead for non-peer fribidi:
#OS_INCLUDES += $(shell pkg-config --cflags --silence-errors fribidi)

# Try to find where NSIS is installed, if anywhere
# First check to see if it's in the path.  If not, check default install path.
NSIS_ROOT ?= $(shell dirname "`which makensis`" | sed "s/\ /\\\ /g")
ifeq ($(NSIS_ROOT), .)
	NSIS_ROOT := /c/Program\ Files/NSIS
endif

# Compiler flags
# requires the commctrl.dll from ie4.0 or greater
DEFINES		+= -D_WIN32_IE=0x0400 -DSUPPORTS_UT_IDLE -DENABLE_SPELL -DENABLE_PRINT
OS_CFLAGS += -mthreads  # required to handle exceptions properly then threads are involved

# Shared library flags
MKSHLIB			= $(LD) --dll

# Compiler options for static and dynamic linkage
STATIC_FLAGS		= -static
SHARED_FLAGS		= -shared -Wl,--no-keep-memory


ABI_NATIVE	= win
ABI_FE		= Win32

##################################################################
## ABIPKGDIR defines the directory containing the Makefile to use to
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)
## This is relative to $(ABI_ROOT)/src/pkg

ABIPKGDIR	= win/setup

# End of mingw defs
