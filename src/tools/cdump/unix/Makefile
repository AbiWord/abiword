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

include $(ABI_ROOT)/src/config/abi_defs.mk

OBJS=			$(OBJDIR)/cdump.$(OBJ_SUFFIX)

PROGRAM=		$(BINDIR)/cdump$(EXE_SUFFIX)

ABI_APPLIBS=		
ABI_APPLIBDEP=		

# libdl flags go into the process later
ABI_LIBS=

# include both dynamic and static binaries
TARGETS=		$(PROGRAM)

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

# One build gets dynamic libdl flags, the other gets static flags
DYNAMIC_LDFLAGS=	$(addprefix -l,$(DL_LIBS))

# we override the $(PROGRAM) rule.

$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP)
	@$(MAKE_OBJDIR)
	@echo Linking dynamic $(PROGRAM)
	@$(CCC) -o $@ $(CFLAGS) $(OBJS) $(DYNAMIC_LDFLAGS) -L/usr/local/lib $(LDFLAGS) $(EXTRA_LIBS) $(XLDFLAGS)

build:: $(TARGETS)
	@echo cdump built
