#! gmake

## AbiWord
## Copyright (C) 1998-2000 AbiSource, Inc.
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
LIBRARY_NAME=		AbiWPAp
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

PLATFORM_OBJS=	$(OBJDIR)/ap_$(ABI_FE)App.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ap_$(ABI_FE)Clipboard.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)DialogFactory.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_WordCount.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Field.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Goto.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Lists.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Options.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_PageNumbers.$(OBJ_SUFFIX)  \
		$(OBJDIR)/ap_$(ABI_FE)Dialog_PageSetup.$(OBJ_SUFFIX)    \
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Paragraph.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Spell.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Styles.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Columns.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Tab.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Frame.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)LeftRuler.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)Prefs.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)StatusBar.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)Toolbar_StyleCombo.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Toolbar_ZoomCombo.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_ToggleCase.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)_TB_CFactory.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)TopRuler.$(OBJ_SUFFIX)


ifeq ($(ABI_FE), Mac)
PLATFORM_OBJS += $(OBJDIR)/ap_$(ABI_FE)Dlg_Replace.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dlg_Break.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dlg_Insert_DateTime.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Tlbr_SizeCombo.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Tlbr_FontCombo.$(OBJ_SUFFIX)
else
PLATFORM_OBJS += $(OBJDIR)/ap_$(ABI_FE)Dialog_Replace.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Break.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Dialog_Insert_DateTime.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Toolbar_SizeCombo.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)Toolbar_FontCombo.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_FE), Unix)
PLATFORM_OBJS += 	$(OBJDIR)/ap_$(ABI_FE)ViewListener.$(OBJ_SUFFIX)

ifdef ABI_OPT_GNOME
PLATFORM_OBJS+=	$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)App.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Break.$(OBJ_SUFFIX) 		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Columns.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Field.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Goto.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Insert_DateTime.$(OBJ_SUFFIX)   \
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Replace.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_WordCount.$(OBJ_SUFFIX)         \
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_PageNumbers.$(OBJ_SUFFIX)       \
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_PageSetup.$(OBJ_SUFFIX)       \
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Paragraph.$(OBJ_SUFFIX)         \
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Lists.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_ToggleCase.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Spell.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Styles.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Tab.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Options.$(OBJ_SUFFIX)
endif
endif

OBJS=		$(PLATFORM_OBJS)				\
		$(OBJDIR)/ap_Convert.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Dialog_Break.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Dialog_WordCount.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Dialog_Field.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Dialog_Goto.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Dialog_Insert_DateTime.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Dialog_Lists.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Dialog_Options.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Dialog_PageNumbers.$(OBJ_SUFFIX)   \
		$(OBJDIR)/ap_Dialog_PageSetup.$(OBJ_SUFFIX)     \
		$(OBJDIR)/ap_Dialog_Paragraph.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Dialog_Replace.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Dialog_Spell.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Dialog_Styles.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Dialog_Columns.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Dialog_ToggleCase.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Dialog_Tab.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_EditMethods.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_FrameData.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LeftRuler.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LoadBindings.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_DeadAbovedot.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_LB_DeadAcute.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_DeadBreve.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_DeadCaron.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_DeadCedilla.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_LB_DeadCircumflex.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_LB_DeadDiaeresis.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_LB_DeadDoubleacute.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_LB_DeadGrave.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_DeadMacron.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_LB_DeadOgonek.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_LB_DeadTilde.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_Default.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_Emacs.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_EmacsCtrlX.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_LB_viEdit.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_viEdit_colon.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_LB_viEdit_c.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_viEdit_d.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_viEdit_r.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_viEdit_y.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_LB_viInput.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Menu_ActionSet.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Menu_Functions.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Menu_LabelSet.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Menu_Layouts.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Prefs.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Preview_Paragraph.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Ruler.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_StatusBar.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Strings.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_Toolbar_ActionSet.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Toolbar_Functions.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Toolbar_Icons.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Toolbar_LabelSet.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_Toolbar_Layouts.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ap_TopRuler.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_wp_splash.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_wp_sidebar.$(OBJ_SUFFIX)


include $(ABI_ROOT)/src/config/abi_rules.mk


build:: $(TARGETS)
