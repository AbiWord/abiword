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

CSRCS=			askrm_Win32.c

PROGRAM=		$(BINDIR)/askrm.exe

ABI_LDFLAGS=		/nologo /subsystem:windows /incremental:no /machine:$(LINK_ARCH) /profile
ABI_APPLIBS=		AbiSetup_Small 

ABI_OTHLIBS=		
####ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
####			shell32 ole32 oleaut32 uuid odbc32 odbccp32
ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
			shell32 uuid comctl32 ole32

GARBAGE=		askrm.map askrm.pdb
TARGETS=		$(PROGRAM)

INCLUDES=		-I$(ABI_XX_ROOT)/src/ask/lib/xp

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

$(PROGRAM): $(OBJS) $(THE_LIBS)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	@$(CC) $(shell echo $(OBJS) | $(TRANSFORM_TO_DOS_PATH))	\
		-Fe$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH))	\
		-link $(LDFLAGS) $(OS_LIBS) $(ABI_LDFLAGS)				\
		$(shell echo $(EXTRA_LIBS) | $(TRANSFORM_TO_DOS_PATH))

build:: $(TARGETS)
	@echo askrm built
