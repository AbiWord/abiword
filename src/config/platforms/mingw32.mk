#! gmake

## AbiSource Program Utilities
## Copyright (C) 2002 AbiSource, Inc.
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
## Mingw platform defines
##############################################################################

# Here for syntactic purposes.
##
PLATFORM_FLAGS		=
PORT_FLAGS		=
OS_CFLAGS		= -fvtable-thunks  # required for compiling ole dragndrop
##

# mingw doesn't like -ansi in compiling wv
ABI_OPT_PACIFY_COMPILER=1
# *** We do not support ancient mingw versions of the gcc2 kind ***

# Now we include some really common stuff.
include $(ABI_ROOT)/src/config/platforms/nix-common.mk
# I highly suggest that here we redefine the compiler, archiver.
# Just need to find a consistent solution, possibly of conditionals,
# that works across mingw installations that have a high internal variance.

# Define tools - resource compiler
RC		= windres

# Suffixes
EXE_SUFFIX = .exe

# Compiler flags
# requires the commctrl.dll from ie4.0 or greater
DEFINES		+= -D_WIN32_IE=0x0400 -DSUPPORTS_UT_IDLE
G++INCLUDES=
GLIB_CONFIG=
GTK_CONFIG=
GNOME_CONFIG=
LIBXML_CONFIG=


# Shared library flags
MKSHLIB			= $(LD) --dll

# Compiler options for static and dynamic linkage
STATIC_FLAGS		= -static
SHARED_FLAGS		= -shared -Wl,--no-keep-memory


ABI_NATIVE	= win
ABI_FE		= Win32

##################################################################
## ABIPKGDIR defines the directory containing the Makefile to use to
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)
## This is relative to $(ABI_ROOT)/src/pkg

ABIPKGDIR	= win/setup

# End of mingw defs
