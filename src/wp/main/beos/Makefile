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

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs_wp.mk

CPPSRCS=		BeOSMain.cpp
PROGRAM=		$(BINDIR)/AbiWord

#TF As of R4 we don't need zlib or libpng any more
#LoadOrder=              AbiGr AbiFmt AbiPTbl AbiImpExp MsWordView AbiXAp AbiWPAp \
#			AbiEv AbiUtil AbiWordVer AbiSpell Abi_expat Abi_libpng Abi_zlib
LoadOrder=              AbiGr AbiFmt AbiPTbl AbiImpExp MsWordView AbiXAp AbiWPAp \
			AbiEv AbiUtil AbiWordVer AbiSpell Abi_expat

## TODO verify that we still need stdc++ for BeOS since we got rid of c++ iostreams

ABI_APPLIBS=		$(LoadOrder)
ABI_LIBS=		translation be tracker png z

ifeq ($(OS_ARCH), x86)
ABI_APPLIBS+=		$(LoadOrder) $(LoadOrder)
ABI_LIBS+=		stdc++.r4
else		#OS_ARCH ppc or BeBox
ABI_LIBS+=	 	mslcpp
LDFLAGS += 		-msg nodup
endif

TARGETS=		$(PROGRAM)

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

# we override the $(PROGRAM) rule.
RESOURCES=$(shell ls *.rsrc)
define ATTACH_RESOURCE
for i in xxxx; do echo "Adding Resource $$i"; xres -o tmprsrc.in $$i; copyres tmprsrc.in $(@); done; listres $(@); rm -f tmprsrc.in
endef

$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	$(CCC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) $(EXTRA_LIBS) $(XLDFLAGS) 
	@xres -o $@ $(RESOURCES)
	@listres $@

build:: $(TARGETS)

