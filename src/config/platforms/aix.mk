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
## IBM AIX platform defines, courtesy of Philippe Defert 
## (Philippe.Defert@cern.ch)
##############################################################################

# Here for syntactic purposes
##
PLATFORM_FLAGS		=
PORT_FLAGS		=
OS_CFLAGS		=
##

# Now we include some really common stuff.
include $(ABI_ROOT)/src/config/platforms/nix-common.mk

## IMPORTANT: Do not edit anything above here, or anything included above here.
## 	      You can redefine anything and everything below this point.

OS_ARCH		:= $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ | sed "s/\//-/")
OS_ENDIAN	= BigEndian32

# Compiler flags
PLATFORM_FLAGS		+= 
PORT_FLAGS		+= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE
OS_CFLAGS		+= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

# Shared library flags
# Are you kidding me?  This conflicts with IBM's documentation,
# but I'll leave it for now Just In Case (tm).
MKSHLIB			= 

# Compiler options for static and dynamic linkage
DL_LIBS			=
STATIC_FLAGS		= -static
LDFLAGS			= -Wl,-bbigtoc

##################################################################              
## ABIPKGDIR defines the directory containing the Makefile to use to            
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)           
## This is relative to $(ABI_ROOT)/src/pkg                                      

ABIPKGDIR       = aix

# End of AIX defs
