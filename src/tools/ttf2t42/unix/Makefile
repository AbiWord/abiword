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

ABI_ROOT:=$(shell pwd)/../../..
include $(ABI_ROOT)/src/config/abi_defs_wp.mk


CSRCS= ttf2t42.c ps.c parse.c util.c encodings.c

build:: $(TARGETS)

OBJS=			$(OBJDIR)/ttf242.$(OBJ_SUFFIX) \
			$(OBJDIR)/ps.$(OBJ_SUFFIX) \
			$(OBJDIR)/parse.$(OBJ_SUFFIX) \
			$(OBJDIR)/util.$(OBJ_SUFFIX) \
			$(OBJDIR)/encodings.$(OBJ_SUFFIX)

PROGRAM=		$(BINDIR)/ttf2t42$(EXE_SUFFIX)

TARGETS=		$(OBJS) $(PROGRAM)


include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

$(PROGRAM): $(OBJS)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)
	@$(CCC) -o $@ $(CFLAGS) $(OBJS)

build:: $(TARGETS)
	@echo ttf2t42 built
