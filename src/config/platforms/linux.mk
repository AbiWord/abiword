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
## Linux platform defines
##############################################################################

##################################################################
##################################################################
## The main makefile and/or this file requires that OS_ARCH be set
## to something to describe which chip that this OS is running on.
## This can be used to change which tools are used and/or which
## compiler/loader options are used.  It will probably also be used
## in constructing the name object file destination directory.

OS_ARCH		:= $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ | sed "s/\//-/")


# Define architecture-specific flags (L. Kollar, 3 Nov 1998)
# These are (probably) optional for your platform.

# Compiler defaults should be fine for Intel.
i386_ARCH_FLAGS		=

# Jerry LeVan <levan@eagle.eku.edu> provided the PPC flags
# Gary Thomas <gdt@linuxppc.org> suggests using -fno-schedule-insns2 
# for some EGCS builds
PPC_ARCH_FLAGS		= -fsigned-char -fno-schedule-insns2

ALPHA_ARCH_FLAGS 	= 
SPARC_ARCH_FLAGS 	= 

# Define tools
CC		= gcc
CCC		= g++
RANLIB		= ranlib

# Suffixes
OBJ_SUFFIX	= o
LIB_SUFFIX	= a
DLL_SUFFIX	= so
AR		= ar cr $@

# Compiler flags
# NOTE:  Using both "-Wall" and "-W" turn on every single darn message
# NOTE:  GCC can throw, and it turns quiet compiles into raging rivers of
# NOTE:  warnings.  -Wall includes the very useful warnings, -W includes
# NOTE:  more stylistic warnings.  -pedantic just gets really picky about
# NOTE:  ANSI things.
ifdef ABI_OPT_DEBUG
OPTIMIZER	= -g -Wall -W -ansi -pedantic
DEFINES		= -DDEBUG -UNDEBUG
OBJ_DIR_SFX	= DBG
else
# NOTE:  In some instances, GCC can only know about truly unused variables
# NOTE:  when optimizations are enabled in the compilation.  For this reason,
# NOTE:  building with optimizations may reveal further warnings not 
# NOTE:  visible without any -O[number] option.
OPTIMIZER	= -O2 -Wall -W -ansi -pedantic
DEFINES		=
OBJ_DIR_SFX	= OBJ
endif

# Includes
OS_INCLUDES		=
G++INCLUDES		= -I/usr/include/g++

# Compiler flags
PLATFORM_FLAGS		= -pipe -DLINUX -Dlinux
PORT_FLAGS		= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE -D__USE_XOPEN_EXTENDED
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

# Architecture-specific flags
ifeq ($(OS_ARCH), i386)
PLATFORM_FLAGS		+= $(i386_ARCH_FLAGS)
OS_ENDIAN		= LittleEndian32
endif

ifeq ($(OS_ARCH), ppc)
PLATFORM_FLAGS		+= $(PPC_ARCH_FLAGS)
OS_ENDIAN		= BigEndian32
endif

ifeq ($(OS_ARCH), alpha)
PLATFORM_FLAGS		+= $(ALPHA_ARCH_FLAGS)
OS_ENDIAN		= LittleEndian32
endif

ifeq ($(OS_ARCH), sparc)
PLATFORM_FLAGS		+= $(SPARC_ARCH_FLAGS)
OS_ENDIAN		= BigEndian32
endif

# Endian for M68K from Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de>
ifeq ($(OS_ARCH), m68k)
PLATFORM_FLAGS		+= $(M68K_ARCH_FLAGS)
OS_ENDIAN		= BigEndian32
endif

GLIB_CONFIG		= glib-config
GTK_CONFIG		= gtk-config
GNOME_CONFIG    	= gnome-config

# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)

# Which links can this platform create.  Define one or
# both of these options.
UNIX_CAN_BUILD_DYNAMIC=1
UNIX_CAN_BUILD_STATIC=1

# Compiler options for static and dynamic linkage
DL_LIBS			= dl
STATIC_FLAGS		= -static

ABI_NATIVE	= unix
ABI_FE		= Unix

##################################################################
## Here you can choice if you want to use the gnome stuff.
## Set ABI_OPT_GNOME to 1 (when invoking 'make') or set as
## an environment variable.
##
## ABI_OPT_GNOME=1

##################################################################
## ABIPKGDIR defines the directory containing the Makefile to use to
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)
## This is relative to $(ABI_ROOT)/src/pkg

ABIPKGDIR	= linux

# End of linux defs
