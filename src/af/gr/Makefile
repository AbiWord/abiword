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

ABI_DEPTH=		..
include $(ABI_DEPTH)/config/abi_defs.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiGr
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

# note: we assume that this will be used as a static library
# note: only.  if we want to use it as a shared library, then
# note: we probably need to append $(SHARED_LIBRARY) to TARGETS
# note: and add a line to the intall section which copies it to
# note: the $(DIST)/bin directory on Windows.

PLATFORM_OBJS=	$(ABI_NATIVE)/$(OBJDIR)/gr_$(ABI_FE)Graphics.$(OBJ_SUFFIX)	\
		$(ABI_NATIVE)/$(OBJDIR)/gr_$(ABI_FE)Image.$(OBJ_SUFFIX)

OBJS=		$(PLATFORM_OBJS)					\
		xp/$(OBJDIR)/gr_DrawArgs.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/gr_Graphics.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/gr_Image.$(OBJ_SUFFIX)

include $(ABI_DEPTH)/config/abi_rules.mk


build:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(TARGETS) $(DIST)/lib
