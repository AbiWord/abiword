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
## BeOS platform defines (grafted from Linux)
##############################################################################

# These are (probably) optional for your platform.
i386_ARCH_FLAGS		= 
PPC_ARCH_FLAGS		=

# Define tools
CC		= cc
CCC		= cc
RANLIB		= ranlib

# Suffixes
OBJ_SUFFIX	= o
LIB_SUFFIX	= a
DLL_SUFFIX	= so
AR		= ar cr $@

# Compiler flags
# TODO evaluate the full set of compiler options.
ifdef ABI_OPT_DEBUG
OPTIMIZER	= -g 
DEFINES		= -DDEBUG -UNDEBUG
OBJ_DIR_SFX	= DBG
else
OPTIMIZER	= 
DEFINES		=
OBJ_DIR_SFX	= OBJ
endif

# Includes
OS_INCLUDES		=
G++INCLUDES		= 

# Compiler flags
PLATFORM_FLAGS		= -DNO_SYS_ERRLIST
#PORT_FLAGS		= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE -D__USE_XOPEN_EXTENDED
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

# Architecture-specific flags
ifeq ($(OS_ARCH), BePC)
PLATFORM_FLAGS		+= $(i386_ARCH_FLAGS)
endif

ifeq ($(OS_ARCH), ppc)
PLATFORM_FLAGS		+= $(PPC_ARCH_FLAGS)
endif

# Shared library flags
#MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)

ABI_NATIVE	= beos
ABI_FE		= BeOS

# End of beos defs
