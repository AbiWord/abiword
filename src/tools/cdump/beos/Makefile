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

CSRCS=			../xp/cdump.c

PROGRAM=		$(BINDIR)/cdump

## TODO verify that we still need stdc++ for BeOS since we got rid of c++ iostreams

ifeq ($(OS_ARCH), BePC)
ABI_APPLIBS=		
ABI_LIBS=		stdc++.r4 translation be tracker
else				#OS_ARCH ppc or BeBox
ABI_APPLIBS=		
ABI_LIBS=	 	mslcpp be translation tracker
endif

TARGETS=		$(PROGRAM)

include $(ABI_DEPTH)/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

ifneq ($(OS_ARCH), BePC)
LDFLAGS += -msg nodup
endif

# we override the $(PROGRAM) rule.

$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	$(CCC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) $(EXTRA_LIBS) $(XLDFLAGS) 

build:: $(TARGETS)

