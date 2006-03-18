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

CPPSRCS=		UnixMain.cpp

INCLUDES=		$(GTK_CFLAGS)

PROGRAM=		$(BINDIR)/AbiWord-2.6
# For all I care, we can allow concurrent building of staticly and dynamically linked executables
# I simply aim for consistency with auto*, until this is phased out entirely.
PROGRAM_S=		$(BINDIR)/AbiWord-2.6
# If both build successfully, one overrides the other.

# TODO the Linux linker is fussy about the ordering of libraries.
# TODO in order to get all symbols to resolve, i had to list them
# TODO several times.  see if there is a better way of doing this.

LoadOrder=		AbiGr AbiFmt AbiPTbl AbiImpExp AbiXAp AbiWPAp \
			AbiEv AbiUtil AbiImpExp AbiWordVer AbiSpell Abi_libwv \
			Abi_ttftool

ifeq ($(ABI_REQUIRE_PEER_ICONV),1)
LoadOrder+= Abi_libiconv
endif

ABI_APPLIBS=		$(LoadOrder) $(LoadOrder) $(LoadOrder)
ABI_APPLIBDEP=		$(LoadOrder)

# libdl flags go into the process later
#ifeq (0,1)
ABI_LIBS=		png z
#else
#ABI_LIBS=		png z iconv
#endif

#ifdef __FreeBSD__
#ABI_LIBS+=		iconv intl
#endif # FreeBSD
#ifdef __OpenBSD__
#ABI_LIBS+=		iconv intl
#endif # OpenBSD

TARGETS=		$(PROGRAM)

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

# One build gets dynamic libdl flags, the other gets static flags
DYNAMIC_LDFLAGS=	$(addprefix -l,$(DL_LIBS))
STATIC_LDFLAGS =	$(STATIC_FLAGS)
ifdef __OpenBSD__
STATIC_LDFLAGS += -lintl
endif
EXTRA_LIBS += 
#EXTRA_LIBS += -lefence
# we override the $(PROGRAM) rule.

ifeq ($(UNIX_CAN_BUILD_DYNAMIC),1)
$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP)
	@$(MAKE_OBJDIR)
	@echo Linking dynamic $(PROGRAM)
	@echo $(CCC) -o $@ $(CFLAGS) $(OBJS) $(DYNAMIC_LDFLAGS) $(LDFLAGS) $(EXTRA_LIBS) $(XLDFLAGS)
	@$(CCC) -o $@ $(CFLAGS) $(OBJS) $(DYNAMIC_LDFLAGS) $(LDFLAGS) $(EXTRA_LIBS) $(OS_LIBS) $(XLDFLAGS)
else
	@echo "Not linking dynamic $(PROGRAM)"
	@echo "    UNIX_CAN_BUILD_DYNAMIC not defined for this platform."
ifeq ($(UNIX_CAN_BUILD_STATIC),1)
$(PROGRAM_S): $(OBJS) $(EXTRA_LIBDEP)
	@$(MAKE_OBJDIR)
	@echo Linking static $(PROGRAM_S)
	@$(CCC) -o $@ $(CFLAGS) $(OBJS) $(STATIC_LDFLAGS) -L/usr/local/lib $(LDFLAGS) $(EXTRA_LIBS) $(OS_LIBS) $(XLDFLAGS)
else
	@echo "Not linking static $(PROGRAM_S)"
	@echo "    UNIX_CAN_BUILD_STATIC not true for this platform."
endif
endif


build:: $(TARGETS)
