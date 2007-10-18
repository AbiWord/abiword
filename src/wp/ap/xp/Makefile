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

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs_wp.mk

ABI_WP_INCS+=	/wp/ap/xp	/wp/ap/$(ABI_NATIVE)

CPPSRCS=	ap_Args.cpp			\
		ap_Convert.cpp			\
		ap_Dialog_New.cpp		\
		ap_Dialog_Break.cpp		\
		ap_Dialog_InsertTable.cpp	\
		ap_Dialog_FormatTable.cpp	\
		ap_Dialog_FormatFrame.cpp	\
		ap_Dialog_WordCount.cpp		\
		ap_Dialog_Field.cpp		\
		ap_Dialog_Goto.cpp		\
		ap_Dialog_Insert_DateTime.cpp	\
		ap_Dialog_Lists.cpp             \
		ap_Dialog_Options.cpp		\
		ap_Dialog_PageNumbers.cpp       \
		ap_Dialog_Paragraph.cpp		\
		ap_Dialog_Replace.cpp		\
		ap_Dialog_Spell.cpp		\
		ap_Dialog_Styles.cpp		\
		ap_Dialog_Columns.cpp		\
		ap_Dialog_Tab.cpp		\
		ap_Dialog_PageSetup.cpp         \
		ap_Dialog_ToggleCase.cpp        \
		ap_Dialog_Background.cpp	\
		ap_Dialog_HdrFtr.cpp	\
		ap_Dialog_InsertBookmark.cpp	\
		ap_Dialog_FormatFootnotes.cpp	\
		ap_Dialog_FormatTOC.cpp	\
		ap_Dialog_InsertHyperlink.cpp	\
		ap_Dialog_MarkRevisions.cpp	\
		ap_Dialog_ListRevisions.cpp	\
		ap_Dialog_MergeCells.cpp	\
		ap_Dialog_SplitCells.cpp	\
		ap_Dialog_MailMerge.cpp		\
		ap_Dialog_Latex.cpp		\
		ap_EditMethods.cpp		\
		ap_FrameData.cpp		\
		ap_LeftRuler.cpp		\
		ap_LoadBindings.cpp		\
		ap_LB_DeadAbovedot.cpp		\
		ap_LB_DeadAcute.cpp		\
		ap_LB_DeadBreve.cpp		\
		ap_LB_DeadCaron.cpp		\
		ap_LB_DeadCedilla.cpp		\
		ap_LB_DeadCircumflex.cpp	\
		ap_LB_DeadDiaeresis.cpp		\
		ap_LB_DeadDoubleacute.cpp	\
		ap_LB_DeadGrave.cpp		\
		ap_LB_DeadMacron.cpp		\
		ap_LB_DeadOgonek.cpp		\
		ap_LB_DeadTilde.cpp		\
		ap_LB_Default.cpp		\
		ap_LB_Emacs.cpp			\
		ap_LB_EmacsCtrlX.cpp		\
		ap_LB_viEdit.cpp		\
		ap_LB_viEdit_colon.cpp		\
		ap_LB_viEdit_c.cpp		\
		ap_LB_viEdit_d.cpp		\
		ap_LB_viEdit_r.cpp		\
		ap_LB_viEdit_y.cpp		\
		ap_LB_viInput.cpp		\
		ap_Menu_ActionSet.cpp		\
		ap_Menu_Functions.cpp		\
		ap_Menu_LabelSet.cpp		\
		ap_Menu_Layouts.cpp		\
		ap_Prefs.cpp			\
		ap_Preview_Paragraph.cpp	\
		ap_Preview_Abi.cpp		\
		ap_Ruler.cpp			\
		ap_StatusBar.cpp		\
		ap_Strings.cpp			\
		ap_Toolbar_ActionSet.cpp	\
		ap_Toolbar_Functions.cpp	\
		ap_Toolbar_Icons.cpp		\
		ap_Toolbar_LabelSet.cpp		\
		ap_Toolbar_Layouts.cpp		\
		ap_Frame.cpp			\
		ap_Dialog_Stylist.cpp		\
		ap_App.cpp			\
		ap_TopRuler.cpp

GENCPPSRCS=     $(OBJDIR)/ap_wp_sidebar.cpp

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)

ifeq ($(ABISOURCE_LICENSED_TRADEMARKS),1)

$(OBJDIR)/ap_wp_sidebar.cpp: sidebar_tm.png $(BINDIR)/cdump$(EXE_SUFFIX)
	rm -f $(OBJDIR)/ap_wp_sidebar.cpp
	$(BINDIR)/cdump$(EXE_SUFFIX) sidebar_tm.png g_pngSidebar > $(OBJDIR)/ap_wp_sidebar.cpp

else

$(OBJDIR)/ap_wp_sidebar.cpp: sidebar.png $(BINDIR)/cdump$(EXE_SUFFIX)
	rm -f $(OBJDIR)/ap_wp_sidebar.cpp
	$(BINDIR)/cdump$(EXE_SUFFIX) ./sidebar.png g_pngSidebar > $(OBJDIR)/ap_wp_sidebar.cpp

endif

