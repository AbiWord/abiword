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

# Here for syntactic purposes
##
PLATFORM_FLAGS		=
PORT_FLAGS		=
OS_CFLAGS		=
##

ABI_REQUIRE_PEER_ICONV = 1
include $(ABI_ROOT)/src/config/platforms/nix-common.mk

OS_ENDIAN	= LittleEndian32

# See below on why I do the following:
ABI_OPT_PEER_EXPAT=1

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
PLATFORM_FLAGS		= -pipe -DFREEBSD -DFreeBSD $(OS_INCLUDES)
# sterwill - I've taken out _POSIX_SOURCE because it breaks popen on FreeBSD 4.0
# fjf      - I've taken out _XOPEN_SOURCE as well because it blocks rint
# mg	   - I'm breaking ties with early versions, as we require gtk2 as of 20020615
PORT_FLAGS		= -D_BSD_SOURCE -DHAVE_STRERROR #-D_XOPEN_SOURCE -D_POSIX_SOURCE
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

PLATFORM_FLAGS		+= 
PORT_FLAGS		+= 

GLIB_CONFIG		= pkg-config glib-2.0
GTK_CONFIG		= pkg-config gtk+-2.0
ifeq ($(ABI_OPT_GNOME),1)
GNOME_CONFIG    	= pkg-config gnome-2.0
endif
# For other unices, we do the libxml2 checks here.
# For *BSD, since I don't see any checks, hardwire expat (for now).

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

# Compiler options for static and dynamic linkage
# ld is handled above
STATIC_FLAGS		= -static

##################################################################
## ABIPKGDIR defines the directory containing the Makefile to use to
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)
## This is relative to $(ABI_ROOT)/src/pkg

ABIPKGDIR	= freebsd

__FreeBSD__ = 1 #fix wchar.h stuff

# End of freebsd defs
