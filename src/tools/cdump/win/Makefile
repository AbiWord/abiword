#! gmake

## AbiSource Setup Kit
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

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs.mk

OBJS=			$(OBJDIR)/cdump.$(OBJ_SUFFIX)

PROGRAM=		$(BINDIR)/cdump$(EXE_SUFFIX)

ABI_LDFLAGS=		/nologo /subsystem:console /incremental:no /machine:$(LINK_ARCH) /profile
ABI_APPLIBS=		

ABI_OTHLIBS=		
ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
			shell32 uuid comctl32 ole32

GARBAGE=		cdump.map cdump.pdb
TARGETS=		$(PROGRAM)

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

$(PROGRAM): $(OBJS)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	@$(CC) $(shell echo $(OBJS) | $(TRANSFORM_TO_DOS_PATH))		\
		-Fe$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH))		\
		-link $(LDFLAGS) $(OS_LIBS) $(ABI_LDFLAGS) $(shell echo $(EXTRA_LIBS) | $(TRANSFORM_TO_DOS_PATH))

build:: $(TARGETS)
	@echo cdump built
