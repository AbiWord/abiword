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
## HP-UX platform defines, courtesy of Philippe Defert 
## (Philippe.Defert@cern.ch)
## Updated by Kevin Vajk (kevin_vajk@hp.com) for gcc with HP-UX ld
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
DLL_SUFFIX	= sl
AR		= ar cr $@

HPUX_MAJOR= $(shell uname -r|sed 's/^[^.]*\.\([^.]*\).*/\1/')
# Compiler flags
ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER	= -g -Wall -ansi -pedantic
DEFINES		= -DDEBUG -UNDEBUG
OBJ_DIR_SFX	= DBG
else
OPTIMIZER	= -O2 -Wall -ansi -pedantic
DEFINES		=
OBJ_DIR_SFX	= OBJ
endif

# Includes
G++INCLUDES		= -I/usr/include/g++

ifeq ($(HPUX_MAJOR), 10)
  # Includes
  OS_INCLUDES		= -I/usr/contrib/include -I/usr/local/include \
                          -I/opt/libpng/include -I/opt/zlib/include \
                          -I/opt/fribidi/include
  # Compiler flags
  PLATFORM_FLAGS	= -L/usr/contrib/lib -L/usr/local/lib -L/opt/libpng/lib -L/opt/zlib/lib \
                          -L/opt/fribidi/lib
  PORT_FLAGS		= -DHAVE_STRERROR -D_HPUX_SOURCE -DSETENV_MISSING
  OS_LIBS 		+= -L/opt/libiconv/lib -liconv
else
  # Includes
  OS_INCLUDES		= -I/usr/contrib/include -I/usr/local/include
  # Compiler flags
  PLATFORM_FLAGS	= -L/usr/contrib/lib -L/usr/local/lib
  PORT_FLAGS		= -DHAVE_STRERROR -D_HPUX_SOURCE -DSETENV_MISSING
endif

OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

PLATFORM_FLAGS		+= 
PORT_FLAGS		+= 

GLIB_CONFIG		= glib-config
GTK_CONFIG		= gtk-config
GNOME_CONFIG    	= gnome-config

# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -b -o $(@:$(OBJDIR)/%.sl=%.sl)

# Which links can this platform create.  Define one or
# both of these options.
# (On HP-UX, we *can* build dynamic, but it's safer not to, since other
# systems we distribute HP-UX packages to may not have all the shared
# libraries in the same locations.)
UNIX_CAN_BUILD_DYNAMIC=0
UNIX_CAN_BUILD_STATIC=1

# Compiler options for static and dynamic linkage
DL_LIBS			= 
STATIC_FLAGS		= -Wl,-a,archive_shared

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

ABIPKGDIR       = hpux
PSICONV_PLATFORM_DEFS= CFLAGS='-O2'

# End of HP-UX defs
