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

CPPSRCS=		QNXMain.cpp

INCLUDES=		

PROGRAM=		$(BINDIR)/AbiWord_d
PROGRAM_S=		$(BINDIR)/AbiWord_s

LoadOrder=		AbiGr AbiFmt AbiPTbl AbiImpExp AbiXAp AbiWPAp \
			AbiEv AbiUtil AbiWordVer AbiSpell Abi_libwv \

ifeq ($(ABI_REQUIRE_PEER_ICONV),1)
LoadOrder+= Abi_libiconv
endif

ABI_APPLIBS=		$(LoadOrder) $(LoadOrder) $(LoadOrder)
ABI_APPLIBDEP=		$(LoadOrder)

# libdl flags go into the process later
# We needed libstdc++ at one point, not any more apparently
ABI_LIBS=		ph phrender m  z png Ap
ifndef ABI_REQUIRE_PEER_ICONV
ABI_LIBS+= iconv 
endif

STUPID_STATIC_LIBS= -Wl,-Bstatic -lphS -lpopt 

# include both dynamic and static binaries
TARGETS=		$(PROGRAM) $(PROGRAM_S)

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

# One build gets dynamic libdl flags, the other gets static flags
DYNAMIC_LDFLAGS=	$(addprefix -l,$(DL_LIBS))
STATIC_LDFLAGS =	

# we override the $(PROGRAM) rule.

$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP)
ifeq ($(QNX_CAN_BUILD_DYNAMIC),1)
	@$(MAKE_OBJDIR)
	@echo Linking dynamic $(PROGRAM)
	@$(CCC) -o $@ $(CFLAGS) $(OBJS) $(DYNAMIC_LDFLAGS) -L/lib -L/usr/local/lib $(LDFLAGS) $(EXTRA_LIBS) $(XLDFLAGS)-lstdc++ $(STUPID_STATIC_LIBS)
	@bindres -q $@ Icon.wgti $(ABI_ROOT)/src/wp/ap/qnx/dialogs/wgt/*.wgtd $(ABI_ROOT)/src/af/xap/qnx/dialogs/wgt/*.wgtd 
else
	@echo "Not linking dynamic $(PROGRAM)"
	@echo "    QNX_CAN_BUILD_DYNAMIC not defined for this platform."
endif

$(PROGRAM_S): $(OBJS) $(EXTRA_LIBDEP)
ifeq ($(QNX_CAN_BUILD_STATIC),1)
	@$(MAKE_OBJDIR)
	@echo Linking static $(PROGRAM_S)
	@$(CCC) -o $@ $(CFLAGS) $(OBJS) $(STATIC_LDFLAGS) -L/lib -L/usr/local/lib $(LDFLAGS) $(EXTRA_LIBS) $(XLDFLAGS) $(STUPID_STATIC_LIBS)
	@bindres -q $@ Icon.wgti $(ABI_ROOT)/src/wp/ap/qnx/dialogs/wgt/*.wgtd $(ABI_ROOT)/src/af/xap/qnx/dialogs/wgt/*.wgtd
else
	@echo "Not linking static $(PROGRAM)"
	@echo "    QNX_CAN_BUILD_STATIC not defined for this platform."
endif

build:: $(TARGETS)
