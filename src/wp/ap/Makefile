#! gmake

## AbiWord
## Copyright (C) 1998 AbiSource, Inc.
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Library General Public
## License as published by the Free Software Foundation; either
## version 2 of the License, or (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Library General Public License for more details.
##
## You should have received a copy of the GNU Library General Public
## License along with this library; if not, write to the
## Free Software Foundation, Inc., 59 Temple Place - Suite 330,
## Boston, MA 02111-1307, USA.

ABI_DEPTH=		../..
include $(ABI_DEPTH)/config/abi_defs.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiAp
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

# note: we assume that this will be used as a static library
# note: only.  if we want to use it as a shared library, then
# note: we probably need to append $(SHARED_LIBRARY) to TARGETS
# note: and add a line to the intall section which copies it to
# note: the $(DIST)/bin directory on Windows.

ifeq ($(ABI_NATIVE), win)
PLATFORM_OBJS=
endif
ifeq ($(ABI_NATIVE), unix)
PLATFORM_OBJS=	unix/$(OBJDIR)/ap_UnixAp.$(OBJ_SUFFIX)			\
		unix/$(OBJDIR)/ap_UnixFrame.$(OBJ_SUFFIX)
endif

OBJS=		$(PLATFORM_OBJS)					\
		xp/$(OBJDIR)/ap_Ap.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_EditMethods.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_Frame.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_LoadBindings.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LoadBindings_Default.$(OBJ_SUFFIX)	\
		xp/$(OBJDIR)/ap_Menu_ActionSet.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_Menu_Functions.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_Menu_LabelSet.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_Menu_Layouts.$(OBJ_SUFFIX)

include $(ABI_DEPTH)/config/abi_rules.mk


libs:: $(TARGETS)

install:: libs
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(TARGETS) $(DIST)/lib
