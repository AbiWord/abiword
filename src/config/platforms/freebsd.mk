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
## FreeBSD platform defines
##############################################################################

##################################################################
##################################################################
## The main makefile and/or this file requires that OS_ARCH be set
## to something to describe which chip that this OS is running on.
## This can be used to change which tools are used and/or which
## compiler/loader options are used.  It will probably also be used
## in constructing the name object file destination directory.

OS_ARCH		:= $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ | sed "s/\//-/")
OS_ENDIAN	= LittleEndian32

ABI_OPT_PEER_EXPAT=1 # See below

# Define tools
CC		= gcc
CCC		= g++
RANLIB		= ranlib

# Suffixes
OBJ_SUFFIX	= o
LIB_SUFFIX	= a
DLL_SUFFIX	= so
AR		= ar cr $@

ifeq ($(ABI_OPT_PANGO),1)
OBJ_DIR_SFX	= PANGO_
else
OBJ_DIR_SFX	=
endif

DEFINES		=
OPTIMIZER	= 

ifeq ($(ABI_OPT_PROF),1)
OPTIMIZER	= -pg -fprofile-arcs -ftest-coverage
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)PRF_
ABI_OPT_OPTIMIZE= 1
ABI_OPTIONS	+= Profile:On
endif

ifeq ($(ABI_OPT_OPTIMIZE),1)
OPTIMIZER	+= -O3
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)OPT_
ABI_OPTIONS	+= Optimize:On
ABI_OPT_DEBUG	= 0
else
OPTIMIZER	= -O2
endif

ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER	= -g
DEFINES		= -DDEBUG
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)DBG_
endif
ifeq ($(ABI_OPT_DEBUG),2)
OPTIMIZER	= -g3 -ggdb3
DEFINES		= -DDEBUG -DUT_DEBUG -DUT_TEST -DFMT_TEST -DPT_TEST -UNDEBUG
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
else
WARNFLAGS	+= -fpermissive -w
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
PLATFORM_FLAGS		= -pipe -DFREEBSD -DFreeBSD $(OS_INCLUDES)
# sterwill - I've taken out _POSIX_SOURCE because it breaks popen on FreeBSD 4.0
# fjf      - I've taken out _XOPEN_SOURCE as well because it blocks rint
# mg	   - I'm breaking ties with early versions, as we require gtk2 as of 20020615
PORT_FLAGS		= -D_BSD_SOURCE -DHAVE_STRERROR #-D_XOPEN_SOURCE -D_POSIX_SOURCE
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

PLATFORM_FLAGS		+= 
PORT_FLAGS		+= 

GLIB_CONFIG		= glib12-config
GTK_CONFIG		= gtk12-config
ifeq ($(ABI_OPT_GNOME),1)
GNOME_CONFIG    	= gnome-config
endif
# For other unices, libxml2 check is here.
# Currently libxml2 support in BSD is broken, so hardwire expat (for now).

# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)

# ELF versions of FreeBSD no longer need an explicit link to libdl.
# This move to ELF happened around the 3.0 releases.  It's possible
# people are running post-3.0 non-ELF systems or running pre-3.0 ELF
# systems.  We assume 3.0 and later are ELF.
OS_RELEASE_MAJOR	= $(shell uname -r | sed -e "s/-.*//")

# default is no libdl
DL_LIBS = 
#special cases for FreeBSD 1 and FreeBSD 2
ifeq ($(OS_RELEASE_MAJOR), 1)
	DL_LIBS = dl
endif
ifeq ($(OS_RELEASE_MAJOR), 2)
	DL_LIBS = dl
endif

# Which links can this platform create.  Define one or
# both of these options.
UNIX_CAN_BUILD_DYNAMIC=1
UNIX_CAN_BUILD_STATIC=1

# Compiler options for static and dynamic linkage
# ld is handled above
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

ABIPKGDIR	= freebsd
PSICONV_PLATFORM_DEFS= CFLAGS='-O2'

__FreeBSD__ = 1 #fix wchar.h stuff

# End of freebsd defs
