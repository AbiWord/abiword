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

#########################################################################
# grafted from BeOS 
#########################################################################

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs_wp.mk

CPPSRCS=		MacMain.cpp

ifeq ($(ABI_OPT_DEBUG), 1)
PROGRAM_BUNDLE=		$(BINDIR)/AbiWord.debug/Content
else
PROGRAM_BUNDLE=		$(BINDIR)/AbiWord.app/Content
endif

PROGRAM_BUNDLE_BIN=$(PROGRAM_BUNDLE)/MacOS
PROGRAM_BUNDLE_RES=$(PROGRAM_BUNDLE)/Resources

PROGRAM_RES=$(PROGRAM_BUNDLE_RES)/AbiWord.rsrc
PROGRAM=$(PROGRAM_BUNDLE_BIN)/AbiWord

LoadOrder=              AbiGr AbiFmt AbiPTbl AbiImpExp AbiXAp AbiWPAp \
			AbiEv AbiUtil AbiWordVer AbiSpell Abi_libwv Abi_libiconv


ABI_APPLIBS=		$(LoadOrder)
ABI_LIBS=		png z

ABI_APPLIBS+=		$(LoadOrder) $(LoadOrder)
ABI_LIBS+=		

TARGETS=		$(PROGRAM)

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

# we override the $(PROGRAM) rule.
RESOURCES=$(shell ls $(OBJDIR)/*.rsrc)

$(PROGRAM_RES): $(RESOURCES)
	@$(MAKE_OBJDIR)
	@echo Creating $(PROGRAM_RES)....
	@$(RESMERGER) -srcIs DF -dstIs DF $(RESOURCES) -o $@
	@cp bundle/PkgInfo $(PROGRAM_BUNDLE)
	@cp bundle/Info.plist $(PROGRAM_BUNDLE)
	@cp bundle/AbiWord.icns $(PROGRAM_BUNDLE_RES)
	@cp bundle/abiword_48.tif $(PROGRAM_BUNDLE_RES)
        

$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP) $(PROGRAM_RES) 
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	$(CCC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) $(EXTRA_LIBS) $(XLDFLAGS) 

build:: $(TARGETS)

