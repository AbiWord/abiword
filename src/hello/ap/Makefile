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

ABI_DEPTH=		../..

include $(ABI_DEPTH)/config/abi_defs_hello.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiHEAp
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

PLATFORM_OBJS=	$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)App.$(OBJ_SUFFIX) \
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)_TB_CFactory.$(OBJ_SUFFIX) \
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Prefs.$(OBJ_SUFFIX) \
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)DialogFactory.$(OBJ_SUFFIX) \
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Frame.$(OBJ_SUFFIX) \
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)StatusBar.$(OBJ_SUFFIX)


OBJS=		$(PLATFORM_OBJS) \
		xp/$(OBJDIR)/ap_EditMethods.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_FrameData.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_LoadBindings.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_LB_Default.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Menu_ActionSet.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Menu_Functions.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Menu_LabelSet.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Menu_Layouts.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Strings.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Toolbar_ActionSet.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Toolbar_Icons.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Toolbar_LabelSet.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Toolbar_Layouts.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_Prefs.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_View.$(OBJ_SUFFIX) \
		xp/$(OBJDIR)/ap_StatusBar.$(OBJ_SUFFIX)


include $(ABI_DEPTH)/config/abi_rules.mk


build:: $(TARGETS)
