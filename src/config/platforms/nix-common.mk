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
## Common Unix(-like) platform defines
##############################################################################

## If a platform ever needs to differ in some way or other, and you have
## doubts regarding the right conditional or if there is one, simply redefine
## in that platform's .mk in this directory.  I'll keep a close eye on this to
## make sure it doesn't get out of hand.  If it does, it'll be scrapped.  -MG

##################################################################
##################################################################
## The main makefile and/or this file requires that OS_ARCH be set
## to something to describe which chip that this OS is running on.
## This can be used to change which tools are used and/or which
## compiler/loader options are used.  It will probably also be used
## in constructing the name object file destination directory.

ifndef $(OS_ARCH)
OS_ARCH		:= $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc32/ -e s/sparc32/sparc/ -e s/arm.*/arm/ -e s/sa110/arm/ | sed "s/\//-/")
endif
ifndef $(OS_REALARCH)
OS_REALARCH	:= $(shell uname -m)
endif

# Define architecture-specific flags (L. Kollar, 3 Nov 1998)
# These are (probably) optional for your platform.

# Compiler defaults should be fine for Intel.
i386_ARCH_FLAGS		=

# Jerry LeVan <levan@eagle.eku.edu> provided the PPC flags
# Gary Thomas <gdt@linuxppc.org> suggests using -fno-schedule-insns2
# for some EGCS builds
# Those no longer are able to build our deps anyway, and thus there's no reason
# not to gain the optimization of a second instruction scheduler pass.

PPC_ARCH_FLAGS		= -fsigned-char

ALPHA_ARCH_FLAGS 	=
SPARC_ARCH_FLAGS 	=
IA64_ARCH_FLAGS		=
S390_ARCH_FLAGS		=

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

ifeq ($(ABI_OPT_PANGO),1)
	OBJ_DIR_SFX	= PANGO_
else
	OBJ_DIR_SFX	=
endif

DEFINES		=
OPTIMIZER	=

ifdef ABI_OPT_PROF
    ifeq ($(ABI_OPT_PROF),1)
    OPTIMIZER   	= -pg
    OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)PRF_
    ABI_OPT_OPTIMIZE= 1
    ABI_OPT_DEBUG	= 0
    ABI_OPTIONS	+= Profile:On
    endif
    # I have decided to minimize support for gcc2, as in order to fully support both gcc2 and gcc3 I would have to make this file huge with option sets.  Sorry.
    # I don't think any of these are really incompatible with gcc2, just that they don't do as much.  Rather, gcc3 combined some options that gcc2 left separate.
    ifeq ($(ABI_OPT_PROF),2)  # WARNING: This is for special purposes only.  It is NOT, I repeat NOT, intended for production use.  It should not be documented elsewhere.
    OPTIMIZER   	= -pg -g -fprofile-arcs -ftest-coverage
    OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)PRF_
    ABI_OPT_OPTIMIZE= 1
    ABI_OPT_DEBUG	= 0
    ABI_OPTIONS	+= Profile:On
    endif
    ifeq ($(ABI_OPT_PROF),3)  # The same warning as above applies here.  Anyway, I encountered some minor but significant glitches with the function instrumentation on c++.  People with gcc 3.1 and earlier may want to avoid this one.  Thus, it is not merged in with level 2.
    OPTIMIZER   	= -pg -g -finstrument-functions -fprofile-arcs -ftest-coverage
    OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)PRF_
    ABI_OPT_OPTIMIZE= 1
    ABI_OPT_DEBUG	= 0
    ABI_OPTIONS	+= Profile:On
    endif
endif

ifeq ($(ABI_OPT_OPTIMIZE),1)
OPTIMIZER	+= -O3
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)OPT_
ABI_OPTIONS	+= Optimize:On
ABI_OPT_DEBUG	= 0
    ifeq ($(ABI_OPT_EXCLUSIVE_OPT),1)
    OPTIMIZER := $(OPTIMIZER) -march=$(OS_REALARCH)
    else
    OPTIMIZER := $(OPTIMIZER) -mcpu=$(OS_REALARCH)
    endif
else
    ifeq ($(ABI_OPT_TINY),1)
    OPTIMIZER	= -Os -fno-default-inline -fno-inline
    else
    OPTIMIZER	= -O2
    endif
endif

ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER	= -g
DEFINES		= -DDEBUG
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)DBG_
endif
ifeq ($(ABI_OPT_DEBUG),2)
OPTIMIZER	= -g3 -ggdb3
DEFINES		= -DDEBUG -DUT_DEBUG -DFMT_TEST -DUT_TEST -DPT_TEST -UNDEBUG
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)DBG_
endif


ifeq ($(ABI_OPT_GNOME),2)
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)GNOME2_
endif	
ifeq ($(ABI_OPT_GNOME),1)
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)GNOME_
endif	
ifeq ($(ABI_OPT_PEER_EXPAT),1)
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)EXP_
endif

OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)OBJ

ifeq ($(ABI_OPT_WAY_TOO_MANY_WARNINGS),1)
WARNFLAGS 	= -Weffc++
else
WARNFLAGS	=
endif

ifneq ($(ABI_OPT_PACIFY_COMPILER),1)
WARNFLAGS	+= -Wall -ansi -pedantic
else
WARNFLAGS	+= -fpermissive -w
endif

# Includes
OS_INCLUDES		=
ifeq ($(ABI_REQUIRE_PEER_ICONV),1)
OS_INCLUDES		+= -I$(ABI_ROOT)/../libiconv/include
endif
G++INCLUDES		= -I/usr/include/g++

# TODO: This is true on about 1/10 platforms.  Why not use pkgconfig?
ifeq ($(ABI_OPT_PANGO),1)
OS_INCLUDES += -I/usr/local/include/pango-1.0 -I/usr/local/include/freetype2 -I/usr/local/include/glib-2.0
endif

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

ifeq ($(OS_ARCH), sparc64)
PLATFORM_FLAGS		+= $(SPARC_ARCH_FLAGS)
OS_ENDIAN		= BigEndian32
endif

# Endian for ARM from Jim Studt <jim@federated.com>
ifeq ($(OS_ARCH), arm)
PLATFORM_FLAGS		+= $(ARM_ARCH_FLAGS)
OS_ENDIAN		= LittleEndian32
endif

# Endian for M68K from Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de>
ifeq ($(OS_ARCH), m68k)
PLATFORM_FLAGS		+= $(M68K_ARCH_FLAGS)
OS_ENDIAN		= BigEndian32
endif

ifeq ($(OS_ARCH), ia64)
PLATFORM_FLAGS		+= $(IA64_ARCH_FLAGS)
OS_ENDIAN		= LittleEndian32
endif

ifeq ($(OS_ARCH), s390)
PLATFORM_FLAGS		+= $(S390_ARCH_FLAGS)
OS_ENDIAN		= BigEndian32
endif

ifeq ($(OS_ARCH), parisc)
PLATFORM_FLAGS      += $(HPPA_ARCH_FLAGS)
OS_ENDIAN       = BigEndian32
endif

ifeq ($(OS_ARCH), parisc64)
PLATFORM_FLAGS      += $(HPPA_ARCH_FLAGS)
OS_ENDIAN       = BigEndian32
endif

# may as well add gthread-2.0 here - fjf
# 
GLIB_CONFIG	= pkg-config glib-2.0 gthread-2.0
GTK_CONFIG	= pkg-config gtk+-2.0 gthread-2.0

# This is wrong.  So far the best option I've seen is gnome-desktop-2.0, but
# that doesn't necessarily catch gnomeprint.  It also puts in libxml2 AFAICT.
# We gotta find the best way to pull it all off real soon.  Until then, I'm commenting it. -MG
# GNOME_CONFIG    = pkg-config gnome-2.0
# AIYEE!  We've merged, and we still dont know...  Also, as our deps are changing rapidly (like gnome-print), I wanna wait for things to calm down.  Worse comes to worse we do separate commands for each submodule.
LIBXML_CONFIG	= xml2-config

# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)
#MKSHLIB			= g++ -shared -o  $(@:$(OBJDIR)/%.so=%.so)


# Which links can this platform create.  Define one or
# both of these options.
UNIX_CAN_BUILD_DYNAMIC=1
# Too many users with the wrong X Extension library, set to 0 as default.
UNIX_CAN_BUILD_STATIC=0

ABI_NATIVE	= unix
ABI_FE		= Unix

