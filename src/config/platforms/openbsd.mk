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

# This sucks
ifneq (,$(shell $(CC) -E - -dM </usr/include/machine/endian.h | grep BYTE_ORDER.*LITTLE_ENDIAN))
OS_ENDIAN      = LittleEndian32
else
OS_ENDIAN      = BigEndian32
endif

# Includes
OS_INCLUDES	+= -I/usr/local/include

# Compiler flags
PLATFORM_FLAGS		+= -pipe -DOPENBSD -DOpenBSD
PORT_FLAGS		+= -DHAVE_STRERROR
OS_CFLAGS		+= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

# Shared library flags redefinition
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)

# Compiler options for static and dynamic linkage
DL_LIBS			= 
STATIC_FLAGS		=  -static

# End of OpenBSD defs

ABIPKGDIR	= openbsd

__OpenBSD__ = 1
