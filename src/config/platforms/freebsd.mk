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
PLATFORM_FLAGS		= -pipe -DFREEBSD -DFreeBSD
PORT_FLAGS		= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR -D_XOPEN_SOURCE
OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

PLATFORM_FLAGS		+= 
PORT_FLAGS		+= 

# Shared library flags
MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)

# Somewhere around the time FreeBSD changed to ELF format binaries,
# the libdl functions got moved into libc (or something like that).
# As a result, -ldl is not neccessary on these newer FreeBSD systems,
# whereas it is needed on older ones.  I'm thinking the split is 
# either 3.0-RELEASE or 3.1-RELEASE.  I _know_ it's needed at 3.1,
# so mail sterwill@abisource.com if I need to be doing this for 3.0
# also.

ifeq ($(OS_RELEASE), 3.1-RELEASE)
	DL_LIBS = 
else
	DL_LIBS = dl
endif

ABI_NATIVE	= unix
ABI_FE		= Unix

# End of linux defs
