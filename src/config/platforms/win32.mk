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
## Win32 platform defines
##############################################################################

# Define tools
CC 	= cl
CCC 	= cl
LINK 	= link
AR 	= lib -NOLOGO -OUT:"$@"
RANLIB 	= echo
BSDECHO	= echo
RC 	= rc.exe

# Windows debugging junk
GARBAGE = $(OBJDIR)/vc20.pdb $(OBJDIR)/vc40.pdb

# Suffixes
OBJ_SUFFIX = obj
LIB_SUFFIX = lib
DLL_SUFFIX = dll

# Compiler flags (do we really need -GT?)
OS_CFLAGS 	= -W3 -nologo -GF -Gy -MDd -GT
OPTIMIZER 	= -Od -Z7
DEFINES 	= -DDEBUG -D_DEBUG -UNDEBUG -D_CRTDBG_MAP_ALLOC -DWIN32 -D_X86_

# note that we only build debug.  TODO
DBG_OR_NOT = DBG

# Shared library flags
DLLFLAGS 	= -DEBUG -DEBUGTYPE:CV -OUT:"$@"
LDFLAGS 	= -DEBUG -DEBUGTYPE:CV
OS_DLLFLAGS 	= -nologo -DLL -SUBSYSTEM:WINDOWS -PDB:NONE

ABI_NATIVE	= win
ABI_FE		= Win32

# End of win32 defs
