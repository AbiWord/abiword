#! gmake

## AbiSource Program Utilities
## Copyright (C) 1998 AbiSource, Inc.
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
## Win32 platform defines
##############################################################################

##################################################################
##################################################################
## The main makefile and/or this file requires that OS_ARCH be set
## to something to describe which chip that this OS is running on.
## This can be used to change which tools are used and/or which
## compiler/loader options are used.  It will probably also be used
## in constructing the name object file destination directory.

OS_ARCH		:= $(shell uname -m | sed -e s/i.86/i386/ | sed "s/\//-/")

# Define tools
CC 	= cl
CCC 	= cl
LINK 	= link
AR 	= lib -NOLOGO -OUT:"$(shell echo $@ | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')"
RANLIB 	= echo
BSDECHO	= echo
RC 	= rc.exe

# Windows debugging junk
GARBAGE = $(OBJDIR)/vc20.pdb $(OBJDIR)/vc40.pdb

# Suffixes
OBJ_SUFFIX = obj
LIB_SUFFIX = lib
DLL_SUFFIX = dll
EXE_SUFFIX = .exe

# Architecture-specific flags
ifeq ($(OS_ARCH), i386)
ARCH_FLAGS	= -D_X86_
OPTIMIZER	= -O2 -Ob1
LINK_ARCH	= IX86
endif

ifeq ($(OS_ARCH), alpha)
ARCH_FLAGS	= -D_ALPHA_
OPTIMIZER	= -Oib3 -QAtuneEV56 -QAarchEV4 -QAgq -QAOu0 -QAieee0
LINK_ARCH	= ALPHA
endif

# Compiler and shared library flags 

ifdef ABI_OPT_DEBUG

OPTIMIZER 	= -Od -Z7 -Ob1
DEFINES 	= -DDEBUG -D_DEBUG -UNDEBUG -D_CRTDBG_MAP_ALLOC
OBJ_DIR_SFX	= DBG
OS_CFLAGS 	= -W3 -nologo -GF -Gy -MDd -DWIN32
DLLFLAGS 	= -DEBUG -DEBUGTYPE:CV -OUT:"$@"
LDFLAGS 	= -DEBUG -DEBUGTYPE:CV
OS_DLLFLAGS 	= -nologo -DLL -SUBSYSTEM:WINDOWS -PDB:NONE

else

DEFINES		= -UDEBUG -U_DEBUG -DNDEBUG
OBJ_DIR_SFX	= OBJ
OS_CFLAGS 	= -W3 -nologo -GF -Gy -MD -DWIN32
DLLFLAGS 	= -OUT:"$@"
LDFLAGS 	=
OS_DLLFLAGS 	= -nologo -DLL -SUBSYSTEM:WINDOWS -PDB:NONE

endif

OS_CFLAGS 	+= $(OPTIMIZER) $(ARCH_FLAGS)

ABI_NATIVE	= win
ABI_FE		= Win32

##################################################################
## ABIPKGDIR defines the directory containing the Makefile to use to
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)
## This is relative to $(ABI_ROOT)/src/pkg

ABIPKGDIR	= win/setup

# End of win32 defs
