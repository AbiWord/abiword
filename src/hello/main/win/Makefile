#! gmake

## AbiHello
## Copyright (C) 1999 AbiSource, Inc.
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

include $(ABI_DEPTH)/config/abi_defs_hello.mk

CPPSRCS=		Win32Main.cpp
RCSRCS=			Win32Main.rc

PROGRAM=		AbiHello.exe

## TODO should /profile be on the following line ??  does this
## TODO enable linking with profile-libraries and thus bloat us.

ABI_LDFLAGS=		/nologo /subsystem:windows /incremental:no /machine:I386 /profile
ABI_APPLIBS=		AbiHelloVer AbiUtil AbiEv AbiXAp AbiGr AbiHEAp Abi_libpng Abi_zlib Abi_expat
ABI_APPLIBDEP=		$(ABI_APPLIBS)

####ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
####			shell32 ole32 oleaut32 uuid odbc32 odbccp32
ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
			shell32 uuid comctl32

GARBAGE=		AbiHello.map AbiHello.pdb
TARGETS=		$(PROGRAM)

include $(ABI_DEPTH)/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	@$(CC) $(subst /,\\,$(OBJS)) -Fe$(subst /,\\,$@) -link $(LDFLAGS) $(OS_LIBS) $(ABI_LDFLAGS) $(subst /,\\,$(EXTRA_LIBS))

build:: $(TARGETS)
	@echo Target $(TARGETS) completed.
