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
## SGI IRIX 64 platform defines, courtesy of Philippe Defert 
## (Philippe.Defert@cern.ch)
##############################################################################
ABI_OPT_PACIFY_COMPILER	= 1 # About equivalent to prior definitions.
include $(ABI_ROOT)/src/config/platforms/nix-common.mk

OS_ENDIAN	= BigEndian32

RANLIB		= true

# Includes
OS_INCLUDES		=

# Compiler flags
PLATFORM_FLAGS		= 
PORT_FLAGS		= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE -DPATH_MAX=1024 -DSCANDIR_MISSING -DSETENV_MISSING
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

UNIX_CAN_BUILD_DYNAMIC=1
UNIX_CAN_BUILD_STATIC=0

# Compiler options for static and dynamic linkage
DL_LIBS			= dl
STATIC_FLAGS		= -static

##################################################################              
## ABIPKGDIR defines the directory containing the Makefile to use to            
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)           
## This is relative to $(ABI_ROOT)/src/pkg                                      

ABIPKGDIR       = irix
PSICONV_PLATFORM_DEFS= CFLAGS='-O2'

EXTRA_LIBS += -liconv

# End of IRIX 64 defs
