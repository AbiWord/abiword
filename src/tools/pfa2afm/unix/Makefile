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


CSRCS= 	pfa2afm.c          \
		splineutil.c       \
		parsepfa.c         \
		splinesaveafm.c    \
		psisolatin1names.c \
		pfautil.c

build:: $(TARGETS)

OBJS=			$(OBJDIR)/pfautil.$(OBJ_SUFFIX)          \
				$(OBJDIR)/pfa2afm.$(OBJ_SUFFIX)          \
				$(OBJDIR)/splineutil.$(OBJ_SUFFIX)       \
				$(OBJDIR)/parsepfa.$(OBJ_SUFFIX)         \
				$(OBJDIR)/splinesaveafm.$(OBJ_SUFFIX)    \
				$(OBJDIR)/psisolatin1names.$(OBJ_SUFFIX)
				

PROGRAM=		$(BINDIR)/pfa2afm$(EXE_SUFFIX)

TARGETS=		$(OBJS) $(PROGRAM)


include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

$(PROGRAM): $(OBJS)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)
	@$(CCC) -o $@ $(CFLAGS) $(OBJS) -lm

build:: $(TARGETS)
	@echo pfa2afm built
