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

# Includes
OS_INCLUDES	+= -I/usr/local/include

# Compiler flags
PLATFORM_FLAGS		= -pipe -DFREEBSD -DFreeBSD $(OS_INCLUDES)
# sterwill - I've taken out _POSIX_SOURCE because it breaks popen on FreeBSD 4.0
# fjf      - I've taken out _XOPEN_SOURCE as well because it blocks rint
# mg	   - I'm breaking ties with early versions, as they no longer build our deps.
PORT_FLAGS		= -D_BSD_SOURCE -DHAVE_STRERROR #-D_XOPEN_SOURCE -D_POSIX_SOURCE
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

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
