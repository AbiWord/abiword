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
include $(ABI_ROOT)/src/config/abi_defs.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiGr
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

PLATFORM_OBJS=	$(OBJDIR)/gr_$(ABI_FE)Graphics.$(OBJ_SUFFIX)	\
		$(OBJDIR)/gr_$(ABI_FE)Image.$(OBJ_SUFFIX)

ifeq ($(ABI_FE), Win32)
PLATFORM_OBJS += $(OBJDIR)/gr_$(ABI_FE)CharWidths.$(OBJ_SUFFIX)
endif
ifeq ($(ABI_FE), Mac)
PLATFORM_OBJS += $(OBJDIR)/gr_$(ABI_FE)Font.$(OBJ_SUFFIX)
endif

OBJS=		$(PLATFORM_OBJS)					\
		$(OBJDIR)/gr_CharWidths.$(OBJ_SUFFIX)			\
		$(OBJDIR)/gr_DrawArgs.$(OBJ_SUFFIX)			\
		$(OBJDIR)/gr_Graphics.$(OBJ_SUFFIX)			\
		$(OBJDIR)/gr_Image.$(OBJ_SUFFIX)			\
		$(OBJDIR)/gr_VectorImage.$(OBJ_SUFFIX)

include $(ABI_ROOT)/src/config/abi_rules.mk


build:: $(TARGETS)
