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
## QNX platform defines ... for Neutrino
##############################################################################

##################################################################
##################################################################
## The main makefile and/or this file requires that OS_ARCH be set
## to something to describe which chip that this OS is running on.
## This can be used to change which tools are used and/or which
## compiler/loader options are used.  It will probably also be used
## in constructing the name object file destination directory.

OS_ARCH		:=$(shell uname -m)

# Compiler defaults should be fine for Intel.
i386_ARCH_FLAGS		= -Vgcc_ntox86_gpp
PPC_ARCH_FLAGS		= # I don't know, don't have a ppc machine.
ARMLE_ARCH_FLAGS  = -Vgcc_ntoarmle_gpp
# Define tools (should gcc be cc/qcc ... same for ar?)
CC		=  qcc 
CCC		=  QCC -Wl,-E 
RANLIB		= touch
LD		=  QCC -Wl,-E
# Suffixes
OBJ_SUFFIX	= o
LIB_SUFFIX	= a
DLL_SUFFIX	= so
AR		= qcc -a $@

# Compiler flags
ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER	= -O0 -g -Wall
DEFINES		= -DDEBUG -UNDEBUG
OBJ_DIR_SFX	= DBG
else
OPTIMIZER	= -O2 -Wall
DEFINES		=
OBJ_DIR_SFX	= OBJ
endif

ifndef ARCH_TARGET
ARCH_TARGET = $(OS_ARCH)
endif

# Includes
OS_INCLUDES	= -I/usr/include

# Architecture-specific flags
ifeq ($(ARCH_TARGET), x86pc)
PLATFORM_FLAGS	+= $(i386_ARCH_FLAGS)
OS_ENDIAN	= LittleEndian32
OBJ_DIR_SFX := $(OBJ_DIR_SFX)_x86pc
endif

ifeq ($(ARCH_TARGET), ppc)
PLATFORM_FLAGS	+= $(PPC_ARCH_FLAGS)
OS_ENDIAN	= BigEndian32
OBJ_DIR_SFX := $(OBJ_DIR_SFX)_ppc
endif

ifeq ($(ARCH_TARGET), armle)
PLATFORM_FLAGS += $(ARMLE_ARCH_FLAGS)
OS_ENDIAN = LittleEndian32
OBJ_DIR_SFX := $(OBJ_DIR_SFX)_armle
endif

# Compiler flags
PORT_FLAGS	=
OS_CFLAGS	= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

# ...

GLIB_CONFIG		=
GTK_CONFIG		=
LIBXML_CONFIG		= xml2-config
# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -shared

# Which links can this platform create.  Define one or
# both of these options.
QNX_CAN_BUILD_DYNAMIC=1
QNX_CAN_BUILD_STATIC=0 #Change if you need to build static. most often this is unecessary.

# Compiler options for static and dynamic linkage
DL_LIBS		=
STATIC_FLAGS	= -static

ABI_NATIVE	=qnx
ABI_FE		=QNX

##################################################################
## ABIPKGDIR defines the directory containing the Makefile to use to
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)
## This is relative to $(ABI_ROOT)/src/pkg

ABIPKGDIR	= qnx

# End of qnx defs
