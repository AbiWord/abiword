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
PROGRAM=		$(BINDIR)/AbiWord.app/Content/MacOS/AbiWord

LoadOrder=              AbiGr AbiFmt AbiPTbl AbiImpExp AbiXAp AbiWPAp \
			AbiEv AbiUtil AbiWordVer AbiSpell Abi_expat Abi_libwv Abi_libiconv


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
RESOURCES=$(shell ls *.rsrc)

$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP) $(RESOURCES)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	$(CCC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) $(EXTRA_LIBS) $(XLDFLAGS) 
# add resource handling for MacOS
#	@xres -o $@ $(RESOURCES)
#	@listres $@

build:: $(TARGETS)

