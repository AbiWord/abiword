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
## Digital UNIX (OSF/1) platform defines, courtesy of Philippe Defert 
## (Philippe.Defert@cern.ch)
##############################################################################

include $(ABI_ROOT)/src/config/platforms/nix-common.mk
OS_ENDIAN	= LittleEndian32 # I don't like the looks of this, but I won't change anything without a box to test on.

# Compiler flags
PLATFORM_FLAGS		= 
PORT_FLAGS		= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE -D_OSF_SOURCE -D_XOPEN_SOURCE_EXTENDED -DAES_SOURCE -DOSF
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

UNIX_CAN_BUILD_DYNAMIC=1
UNIX_CAN_BUILD_STATIC=1

# Compiler options for static and dynamic linkage
# ??? does Digital Unix need dl ???
DL_LIBS			= 
STATIC_FLAGS		= -static

##################################################################              
## ABIPKGDIR defines the directory containing the Makefile to use to            
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)           
## This is relative to $(ABI_ROOT)/src/pkg                                      

ABIPKGDIR       = osf1

# End of OSF/1 defs
