#! gmake

## AbiSource Program Utilities
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

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiEv
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)


PLATFORM_OBJS=	$(OBJDIR)/ev_$(ABI_FE)Keyboard.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ev_$(ABI_FE)Menu.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ev_$(ABI_FE)Mouse.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ev_$(ABI_FE)Toolbar.$(OBJ_SUFFIX)	\

ifeq ($(ABI_FE), BeOS)
PLATFORM_OBJS += $(OBJDIR)/ev_$(ABI_FE)Tooltip.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_FE), Mac)
PLATFORM_OBJS += $(OBJDIR)/ev_$(ABI_FE)Tlbr_ViewListener.$(OBJ_SUFFIX)
else
PLATFORM_OBJS += $(OBJDIR)/ev_$(ABI_FE)Toolbar_ViewListener.$(OBJ_SUFFIX)
endif

ifdef ABI_OPT_GNOME
PLATFORM_OBJS +=	$(OBJDIR)/ev_$(ABI_FE)$(ABI_GNOME_PREFIX)Menu.$(OBJ_SUFFIX)	\
			$(OBJDIR)/ev_$(ABI_FE)$(ABI_GNOME_PREFIX)Toolbar.$(OBJ_SUFFIX)
endif


OBJS=		$(PLATFORM_OBJS)							\
		$(OBJDIR)/ev_EditBinding.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ev_EditEventMapper.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ev_EditMethod.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ev_Keyboard.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ev_Menu.$(OBJ_SUFFIX)				\
		$(OBJDIR)/ev_Menu_Actions.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ev_Menu_Labels.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ev_Menu_Layouts.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ev_Mouse.$(OBJ_SUFFIX)				\
		$(OBJDIR)/ev_NamedVirtualKey.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ev_Toolbar.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ev_Toolbar_Actions.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ev_Toolbar_Control.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ev_Toolbar_Labels.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ev_Toolbar_Layouts.$(OBJ_SUFFIX)

include $(ABI_ROOT)/src/config/abi_rules.mk


build:: $(TARGETS)
