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
ABI_REQUIRE_PEER_ICONV = 1
ABI_OPT_PEER_EXPAT=1 # I promise I'll fix this...sometime.

# Here for syntactic purposes.
##
PLATFORM_FLAGS		=
PORT_FLAGS		=
OS_CFLAGS		=
##

include $(ABI_ROOT)/src/config/platforms/nix-common.mk

# See below on why I do the following:
ABI_OPT_PEER_EXPAT=1

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

GLIB_CONFIG		= pkg-config glib-2.0
GTK_CONFIG		= pkg-config gtk+-2.0
ifeq ($(ABI_OPT_GNOME),1)
GNOME_CONFIG    	= pkg-config gnome-2.0
endif
# For other unices the libxml2 check is here.
# For *BSD, I see no check, so hardwire expat (for now).

# Shared library flags redefinition
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)

# Compiler options for static and dynamic linkage
DL_LIBS			= 
STATIC_FLAGS		=  -static

# End of OpenBSD defs

ABIPKGDIR	= openbsd

__OpenBSD__ = 1
