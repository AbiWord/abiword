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

# Here for syntactic purposes.
##
PLATFORM_FLAGS		=
PORT_FLAGS		=
OS_CFLAGS		=
## 

# Now we include some really common stuff.
include $(ABI_ROOT)/src/config/platforms/nix-common.mk

<<<<<<< linux.mk
ifeq ($(ABI_OPT_PANGO),1)
	OBJ_DIR_SFX	= PANGO_
else
	OBJ_DIR_SFX	=
endif

DEFINES		=
OPTIMIZER	=

ifeq ($(ABI_OPT_PROF),1)
OPTIMIZER   	= -pg -fprofile-arcs -ftest-coverage
OBJ_DIR_SFX	:= $(OBJ_DIR_SFX)PRF_
ABI_OPT_OPTIMIZE= 1
ABI_OPT_DEBUG	= 0
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
endif

# Includes
OS_INCLUDES		=
ifeq ($(ABI_REQUIRE_PEER_ICONV),1)
OS_INCLUDES		+= -I$(ABI_ROOT)/../libiconv/include
endif
G++INCLUDES		= -I/usr/include/g++
ifeq ($(ABI_OPT_PANGO),1)
OS_INCLUDES += -I/usr/local/include/pango-1.0 -I/usr/local/include/freetype2 -I/usr/local/include/glib-2.0
endif
=======
### IMPORTANT: Don't edit anything above here, or anything included above here.
### 	       You can redefine anything and everything below this point.
>>>>>>> 1.71

# Compiler flags
<<<<<<< linux.mk
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

ifeq ($(ABI_OPT_GNOME),2)
GLIB_CONFIG	= pkg-config glib-2.0
GTK_CONFIG	= pkg-config gtk+-2.0
GNOME_CONFIG    = pkg-config gnome-2.0
else
GLIB_CONFIG	= glib-config
GTK_CONFIG	= gtk-config
GNOME_CONFIG    = gnome-config
endif

LIBXML_CONFIG	= xml2-config

# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)
#MKSHLIB			= g++ -shared -o  $(@:$(OBJDIR)/%.so=%.so)


# Which links can this platform create.  Define one or
# both of these options.
UNIX_CAN_BUILD_DYNAMIC=1
# Too many users with the wrong X Extension library, set to 0 as default.
UNIX_CAN_BUILD_STATIC=1
=======
PLATFORM_FLAGS		+= -pipe -DLINUX -Dlinux
PORT_FLAGS		+= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE -D__USE_XOPEN_EXTENDED
OS_CFLAGS		+= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)
>>>>>>> 1.71

# Compiler options for static and dynamic linkage
DL_LIBS			= dl
STATIC_FLAGS		= -static

##################################################################
## ABIPKGDIR defines the directory containing the Makefile to use to
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)
## This is relative to $(ABI_ROOT)/src/pkg

ABIPKGDIR	= linux

# End of linux defs
