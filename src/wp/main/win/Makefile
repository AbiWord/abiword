#! gmake

## AbiWord
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

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

EXPORT_HEADERS=
HEADERS=		$(EXPORT_HEADERS)


RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		Win32Main.cpp

PROGRAM=		AbiWord.exe

ABI_LDFLAGS=		/nologo /subsystem:windows /incremental:no /machine:I386 /profile
ABI_APPLIBS=		AbiVer AbiUtil AbiPS AbiImpExp AbiPTbl AbiFmt AbiEv AbiGr AbiAp

ABI_OTHLIBS=		expat $(ABI_JSLIBS)
####ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
####			shell32 ole32 oleaut32 uuid odbc32 odbccp32
ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
			shell32 uuid

GARBAGE=		AbiWord.map AbiWord.pdb
TARGETS=		$(PROGRAM)

include $(ABI_DEPTH)/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

# ACK!  TODO - do we really want a directory pattern match here in a makefile?

THE_LIBS=		$(DIST)/lib/*

## override $(PROGRAM) target in rules

$(PROGRAM): $(OBJS) $(THE_LIBS)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	@$(CC) $(OBJS) -Fe$@ -link $(LDFLAGS) $(OS_LIBS) $(ABI_LDFLAGS) $(EXTRA_LIBS)

build:: $(TARGETS)
	@echo Exporting $(TARGETS) from `pwd`...
	@$(subst xxxx,$(DIST)/bin,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 755 $(TARGETS) $(DIST)/bin

