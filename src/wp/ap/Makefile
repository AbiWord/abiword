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
include $(ABI_DEPTH)/config/abi_defs_wp.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiWPAp
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

PLATFORM_OBJS=	$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)App.$(OBJ_SUFFIX)				\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)DialogFactory.$(OBJ_SUFFIX)			\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Dialog_Break.$(OBJ_SUFFIX)			\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Dialog_Replace.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Frame.$(OBJ_SUFFIX)				\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)LeftRuler.$(OBJ_SUFFIX)			\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Prefs.$(OBJ_SUFFIX)				\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)StatusBar.$(OBJ_SUFFIX)			\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Toolbar_StyleCombo.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Toolbar_FontCombo.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Toolbar_SizeCombo.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)Toolbar_ZoomCombo.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)_TB_CFactory.$(OBJ_SUFFIX)	\
		$(ABI_NATIVE)/$(OBJDIR)/ap_$(ABI_FE)TopRuler.$(OBJ_SUFFIX)


OBJS=		$(PLATFORM_OBJS)						\
		xp/$(OBJDIR)/ap_Dialog_Break.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Dialog_Replace.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_EditMethods.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_FrameData.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/ap_LeftRuler.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/ap_LoadBindings.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_LB_DeadAbovedot.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_DeadAcute.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_DeadBreve.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_DeadCaron.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_DeadCedilla.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_DeadCircumflex.$(OBJ_SUFFIX)	\
		xp/$(OBJDIR)/ap_LB_DeadDiaeresis.$(OBJ_SUFFIX)	\
		xp/$(OBJDIR)/ap_LB_DeadDoubleacute.$(OBJ_SUFFIX)	\
		xp/$(OBJDIR)/ap_LB_DeadGrave.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_DeadMacron.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_DeadOgonek.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_DeadTilde.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_Default.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LB_Emacs.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_LB_EmacsCtrlX.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_LB_viEdit.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/ap_LB_viEdit_colon.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_LB_viEdit_c.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_LB_viEdit_d.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_LB_viEdit_y.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_LB_viInput.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Menu_ActionSet.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Menu_Functions.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Menu_LabelSet.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Menu_Layouts.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Prefs.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/ap_Ruler.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/ap_StatusBar.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/ap_Strings.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/ap_Toolbar_ActionSet.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Toolbar_Functions.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Toolbar_Icons.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Toolbar_LabelSet.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_Toolbar_Layouts.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_TopRuler.$(OBJ_SUFFIX)

include $(ABI_DEPTH)/config/abi_rules.mk


build:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(TARGETS) $(DIST)/lib
