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
## OpenBSD platform defines, courtesy of kstailey@kstailey.tzo.com
##############################################################################

##################################################################
##################################################################
## The main makefile and/or this file requires that OS_ARCH be set
## to something to describe which chip that this OS is running on.
## This can be used to change which tools are used and/or which
## compiler/loader options are used.  It will probably also be used
## in constructing the name object file destination directory.

OS_ARCH                := $(shell uname -m)

ABI_OPT_PEER_EXPAT=1 # See below

ifneq (,$(shell $(CC) -E - -dM </usr/include/machine/endian.h | grep BYTE_ORDER.*LITTLE_ENDIAN))
OS_ENDIAN      = LittleEndian32
else
OS_ENDIAN      = BigEndian32
endif

# Define tools
CC		= gcc
CCC		= g++
RANLIB		= ranlib

# Suffixes
OBJ_SUFFIX	= o
LIB_SUFFIX	= a
DLL_SUFFIX	= so
AR		= ar cr $@

DEFINES		=
OPTIMIZER	= 

ifeq ($(ABI_OPT_PROF),1)
OPTIMIZER	= -pg -fprofile-arcs -ftest-coverage
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)PRF_
ABI_OPT_OPTIMIZE= 1
ABI_OPTIONS	+= Profile:On
endif

ifeq ($(ABI_OPT_OPTIMIZE),1)
OPTIMIZER	+= -O3 -fomit-frame-pointer
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)OPT_
ABI_OPTIONS	+= Optimize:On
ABI_OPT_DEBUG	= 0
else
OPTIMIZER	= -O2
endif

ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER	= -g -Wall -pedantic -Wno-long-long
ABI_OPT_PACIFY_COMPILER = 1
DEFINES		= -DDEBUG -UNDEBUG
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)DBG_
endif


ifeq ($(ABI_OPT_GNOME),1)
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)GNOME_
endif
ifeq ($(ABI_OPT_PEER_EXPAT),1)
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)EXP_
endif

OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)OBJ

ifeq ($(ABI_OPT_WAY_TOO_MANY_WARNINGS),1)
WARNFLAGS	= -Weffc++
else
WARNFLAGS	=
endif

ifneq ($(ABI_OPT_PACIFY_COMPILER),1)
WARNFLAGS	+= -Wall -ansi -pedantic
endif

ABI_REQUIRE_PEER_ICONV = 1
# Includes
ifeq ($(ABI_REQUIRE_PEER_ICONV),1)
OS_INCLUDES	= -I$(ABI_ROOT)/../libiconv/include -I/usr/local/include
else
OS_INCLUDES	= -I/usr/local/include
endif
G++INCLUDES	= -I/usr/include/g++

# Compiler flags
PLATFORM_FLAGS		= -pipe -DOPENBSD -DOpenBSD
PORT_FLAGS		= -DHAVE_STRERROR
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

PLATFORM_FLAGS		+= 
PORT_FLAGS		+= 

GLIB_CONFIG		= glib-config
GTK_CONFIG		= gtk-config
ifeq ($(ABI_OPT_GNOME),1)
GNOME_CONFIG    	= gnome-config
endif
# For now no libxml2 check, hardwire expat.

# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)

# Which links can this platform create.  Define one or
# both of these options.
UNIX_CAN_BUILD_DYNAMIC=1
UNIX_CAN_BUILD_STATIC=1

##################################################################
## Here you can choice if you want to use the gnome stuff.
## Set ABI_OPT_GNOME to 1 (when invoking 'make') or set as
## an environment variable.
##
## ABI_OPT_GNOME=1

# Compiler options for static and dynamic linkage
DL_LIBS			= 
STATIC_FLAGS		=  -static

ABI_NATIVE	= unix
ABI_FE		= Unix

# End of OpenBSD defs

ABIPKGDIR	= openbsd
PSICONV_PLATFORM_DEFS= CFLAGS='-O2'

__OpenBSD__ = 1
