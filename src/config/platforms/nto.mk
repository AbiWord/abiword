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
i386_ARCH_FLAGS		=
PPC_ARCH_FLAGS		=
ALPHA_ARCH_FLAGS 	= 
SPARC_ARCH_FLAGS 	= 

# Define tools (should gcc be cc/qcc ... same for ar?)
CC		= qcc
CCC		= qcc
RANLIB		= touch

# Suffixes
OBJ_SUFFIX	= o
LIB_SUFFIX	= a
DLL_SUFFIX	= so
AR		= qcc -a $@

# Compiler flags
ifdef ABI_OPT_DEBUG
OPTIMIZER	= -g 
DEFINES		= -DDEBUG -UNDEBUG
OBJ_DIR_SFX	= DBG
else
OPTIMIZER	= -O2 
DEFINES		=
OBJ_DIR_SFX	= OBJ
endif

# Includes
OS_INCLUDES	= -I$(ABI_ROOT)/../libiconv/include
G++INCLUDES	=

# Compiler flags
PLATFORM_FLAGS	= 
#PORT_FLAGS	= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE -D__USE_XOPEN_EXTENDED
PORT_FLAGS	= 
OS_CFLAGS	= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

# Architecture-specific flags
ifeq ($(OS_ARCH), x86pc)
PLATFORM_FLAGS	+= $(i386_ARCH_FLAGS)
OS_ENDIAN	= LittleEndian32
endif

ifeq ($(OS_ARCH), ppc)
PLATFORM_FLAGS	+= $(PPC_ARCH_FLAGS)
OS_ENDIAN	= BigEndian32
endif

# ...

GLIB_CONFIG		=
GTK_CONFIG		=

# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)

# Which links can this platform create.  Define one or
# both of these options.
QNX_CAN_BUILD_DYNAMIC=0
QNX_CAN_BUILD_STATIC=1

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
