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

##################################################################
##################################################################
## The main makefile and/or this file requires that OS_ARCH be set
## to something to describe which chip that this OS is running on.
## This can be used to change which tools are used and/or which
## compiler/loader options are used.  It will probably also be used
## in constructing the name object file destination directory.

OS_ARCH		:= $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ | sed "s/\//-/")
OS_ENDIAN	= BigEndian32

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
# NOTE:  Using both "-Wall" and "-W" turn on every single darn message
# NOTE:  GCC can throw, and it turns quiet compiles into raging rivers of
# NOTE:  warnings.  -Wall includes the very useful warnings, -W includes
# NOTE:  more stylistic warnings.  -pedantic just gets really picky about
# NOTE:  ANSI things.
ifdef ABI_OPT_DEBUG
OPTIMIZER	= -g -Wall -W -ansi -pedantic
DEFINES		= -DDEBUG -UNDEBUG
OBJ_DIR_SFX	= DBG
else
# NOTE:  In some instances, GCC can only know about truly unused variables
# NOTE:  when optimizations are enabled in the compilation.  For this reason,
# NOTE:  building with optimizations may reveal further warnings not 
# NOTE:  visible without any -O[number] option.
OPTIMIZER	= -O2 -Wall -W -ansi -pedantic
DEFINES		=
OBJ_DIR_SFX	= OBJ
endif

# Includes
OS_INCLUDES		=
G++INCLUDES		= -I/usr/include/g++

# Compiler flags
PLATFORM_FLAGS		= 
PORT_FLAGS		= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

PLATFORM_FLAGS		+=
PORT_FLAGS		+= 

GLIB_CONFIG		= glib-config
GTK_CONFIG		= gtk-config
GNOME_CONFIG    	= gnome-config

# Shared library flags
MKSHLIB			= 

# Which links can this platform create.  Define one or
# both of these options. (what can IRIX do?  someone
# with an SGI set the right one and mail a patch)
UNIX_CAN_BUILD_DYNAMIC=1
UNIX_CAN_BUILD_STATIC=0

# Compiler options for static and dynamic linkage
DL_LIBS			=
STATIC_FLAGS		= -static
LDFLAGS			= -Wl,-bbigtoc

ABI_NATIVE	= unix
ABI_FE		= Unix

##################################################################
## Here you can choice if you want to use the gnome stuff.
## Set ABI_OPT_GNOME to 1 (when invoking 'make') to override
## the setting of 0 here.  Example:  "make ABI_OPT_GNOME=1 install"

ABI_OPT_GNOME		= 0
ABI_GNOME_DIR		= gnome
ABI_GNOME_PREFIX	= Gnome

##################################################################              
## ABIPKGDIR defines the directory containing the Makefile to use to            
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)           
## This is relative to $(ABI_ROOT)/src/pkg                                      

ABIPKGDIR       = aix

# End of AIX defs
