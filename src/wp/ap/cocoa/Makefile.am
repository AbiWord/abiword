## AbiSource Applications
## Copyrught (C) 2009 Hubert Figuiere
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

include $(top_srcdir)/src/wp/main/cocoa/bundle.mk

noinst_LTLIBRARIES = libcocoa.la

AM_CPPFLAGS = \
	$(WP_CPPFLAGS) \
	-x objective-c++

#	-I$(top_srcdir)/src/wp/ap/xp/ToolbarIcons 		\
#	-DPREFIX=\""$(prefix)"\" 				\
#	-DSYSCONFDIR=\""$(sysconfdir)"\" 			\
#	-DABIWORD_PLUGINSDIR="\"$(ABIWORD_PLUGINSDIR)"\"

nobase_resources_DATA = \
	ap_CocoaDialog_Annotation.nib/*.nib \
	ap_CocoaDialog_Background.nib/*.nib\
	ap_CocoaDialog_Break.nib/*.nib\
	ap_CocoaDialog_Columns.nib/*.nib\
	ap_CocoaDialog_Field.nib/*.nib\
	ap_CocoaDialog_FormatFootnotes.nib/*.nib\
	ap_CocoaDialog_FormatFrame.nib/*.nib\
	ap_CocoaDialog_FormatTOC.nib/*.nib\
	ap_CocoaDialog_FormatTable.nib/*.nib\
	ap_CocoaDialog_Goto.nib/*.nib\
	ap_CocoaDialog_HdrFtr.nib/*.nib\
	ap_CocoaDialog_InsertBookmark.nib/*.nib\
	ap_CocoaDialog_InsertHyperlink.nib/*.nib\
	ap_CocoaDialog_InsertTable.nib/*.nib\
	ap_CocoaDialog_Latex.nib/*.nib\
	ap_CocoaDialog_ListRevisions.nib/*.nib\
	ap_CocoaDialog_Lists.nib/*.nib\
	ap_CocoaDialog_MailMerge.nib/*.nib\
	ap_CocoaDialog_MarkRevisions.nib/*.nib\
	ap_CocoaDialog_MergeCells.nib/*.nib\
	ap_CocoaDialog_MetaData.nib/*.nib\
	ap_CocoaDialog_New.nib/*.nib\
	ap_CocoaDialog_Options.nib/*.nib\
	ap_CocoaDialog_PageNumbers.nib/*.nib\
	ap_CocoaDialog_PageSetup.nib/*.nib\
	ap_CocoaDialog_Paragraph.nib/*.nib\
	ap_CocoaDialog_Replace.nib/*.nib\
	ap_CocoaDialog_Spell.nib/*.nib\
	ap_CocoaDialog_SplitCells.nib/*.nib\
	ap_CocoaDialog_Styles.nib/*.nib\
	ap_CocoaDialog_StylesModify.nib/*.nib\
	ap_CocoaDialog_Stylist.nib/*.nib\
	ap_CocoaDialog_Tab.nib/*.nib\
	ap_CocoaDialog_ToggleCase.nib/*.nib\
	ap_CocoaDialog_WordCount.nib/*.nib\
	ap_CocoaFrame.nib/*.nib\
	$(NULL)

libcocoa_la_SOURCES = \
	ap_CocoaApp.h \
	ap_CocoaAppImpl.h \
	ap_CocoaAppImpl.cpp \
	ap_CocoaApp.cpp \
	ap_CocoaClipboard.h \
	ap_CocoaClipboard.cpp \
	ap_CocoaDialog_All.h \
	ap_CocoaDialog_Annotation.h \
	ap_CocoaDialog_Annotation.cpp \
	ap_CocoaDialog_Background.h \
	ap_CocoaDialog_Background.cpp \
	ap_CocoaDialog_Break.h \
	ap_CocoaDialog_Break.cpp \
	ap_CocoaDialog_Columns.h \
	ap_CocoaDialog_Columns.cpp \
	ap_CocoaDialogFactory.cpp \
	ap_CocoaDialog_Field.h \
	ap_CocoaDialog_Field.cpp \
	ap_CocoaDialog_FormatFootnotes.h \
	ap_CocoaDialog_FormatFootnotes.cpp \
	ap_CocoaDialog_FormatFrame.h \
	ap_CocoaDialog_FormatFrame.cpp \
	ap_CocoaDialog_FormatTable.h \
	ap_CocoaDialog_FormatTable.cpp \
	ap_CocoaDialog_FormatTOC.h \
	ap_CocoaDialog_FormatTOC.cpp \
	ap_CocoaDialog_Goto.h \
	ap_CocoaDialog_Goto.cpp \
	ap_CocoaDialog_HdrFtr.h \
	ap_CocoaDialog_HdrFtr.cpp \
	ap_CocoaDialog_InsertBookmark.h \
	ap_CocoaDialog_InsertBookmark.cpp \
	ap_CocoaDialog_Insert_DateTime.h \
	ap_CocoaDialog_Insert_DateTime.cpp \
	ap_CocoaDialog_InsertHyperlink.h \
	ap_CocoaDialog_InsertHyperlink.cpp \
	ap_CocoaDialog_InsertTable.h \
	ap_CocoaDialog_InsertTable.cpp \
	ap_CocoaDialog_Latex.h \
	ap_CocoaDialog_Latex.cpp \
	ap_CocoaDialog_ListRevisions.h \
	ap_CocoaDialog_ListRevisions.cpp \
	ap_CocoaDialog_Lists.h \
	ap_CocoaDialog_Lists.cpp \
	ap_CocoaDialog_MailMerge.h \
	ap_CocoaDialog_MailMerge.cpp \
	ap_CocoaDialog_MarkRevisions.h \
	ap_CocoaDialog_MarkRevisions.cpp \
	ap_CocoaDialog_MergeCells.h \
	ap_CocoaDialog_MergeCells.cpp \
	ap_CocoaDialog_MetaData.h \
	ap_CocoaDialog_MetaData.cpp \
	ap_CocoaDialog_New.h \
	ap_CocoaDialog_New.cpp \
	ap_CocoaDialog_Options.h \
	ap_CocoaDialog_Options.cpp \
	ap_CocoaDialog_PageNumbers.h \
	ap_CocoaDialog_PageNumbers.cpp \
	ap_CocoaDialog_PageSetup.h \
	ap_CocoaDialog_PageSetup.cpp \
	ap_CocoaDialog_Paragraph.h \
	ap_CocoaDialog_Paragraph.cpp \
	ap_CocoaDialog_Replace.h \
	ap_CocoaDialog_Replace.cpp \
	ap_CocoaDialog_Spell.h \
	ap_CocoaDialog_Spell.cpp \
	ap_CocoaDialog_SplitCells.h \
	ap_CocoaDialog_SplitCells.cpp \
	ap_CocoaDialog_Styles.h \
	ap_CocoaDialog_Styles.cpp \
	ap_CocoaDialog_Stylist.h \
	ap_CocoaDialog_Stylist.cpp \
	ap_CocoaDialog_Tab.h \
	ap_CocoaDialog_Tab.cpp \
	ap_CocoaDialog_ToggleCase.h \
	ap_CocoaDialog_ToggleCase.cpp \
	ap_CocoaDialog_WordCount.h \
	ap_CocoaDialog_WordCount.cpp \
	ap_CocoaFrame.h \
	ap_CocoaFrameImpl.h \
	ap_CocoaFrameImpl.cpp \
	ap_CocoaFrame.cpp \
	ap_CocoaLeftRuler.h \
	ap_CocoaLeftRuler.cpp \
	ap_CocoaPlugin.h \
	ap_CocoaPlugin.cpp \
	ap_CocoaPrefs.h \
	ap_CocoaPrefs.cpp \
	ap_CocoaPreview_Annotation.h \
	ap_CocoaPreview_Annotation.cpp \
	ap_Cocoa_ResourceIDs.h \
	ap_Cocoa_ResourceIDs.cpp \
	ap_CocoaStatusBar.h \
	ap_CocoaStatusBar.cpp \
	ap_Cocoa_TB_CFactory.cpp \
	ap_CocoaToolbar_Control_All.h \
	ap_CocoaToolbar_FontCombo.h \
	ap_CocoaToolbar_FontCombo.cpp \
	ap_CocoaToolbar_SizeCombo.h \
	ap_CocoaToolbar_SizeCombo.cpp \
	ap_CocoaToolbar_StyleCombo.h \
	ap_CocoaToolbar_StyleCombo.cpp \
	ap_CocoaToolbar_ZoomCombo.h \
	ap_CocoaToolbar_ZoomCombo.cpp \
	ap_CocoaTool.h \
	ap_CocoaTool.cpp \
	ap_CocoaTopRuler.h \
	ap_CocoaTopRuler.cpp \
	ap_CocoaViewListener.h \
	ap_CocoaViewListener.cpp \
	ap_PrintingDelegate.h \
	ap_PrintingDelegate.cpp

EXTRA_DIST = \
	ap_CocoaDialog_Annotation.nib \
	ap_CocoaDialog_Background.nib \
	ap_CocoaDialog_Break.nib \
	ap_CocoaDialog_Columns.nib \
	ap_CocoaDialog_Field.nib \
	ap_CocoaDialog_FormatFootnotes.nib \
	ap_CocoaDialog_FormatFrame.nib \
	ap_CocoaDialog_FormatTOC.nib \
	ap_CocoaDialog_FormatTable.nib \
	ap_CocoaDialog_Goto.nib \
	ap_CocoaDialog_HdrFtr.nib \
	ap_CocoaDialog_InsertBookmark.nib \
	ap_CocoaDialog_InsertHyperlink.nib \
	ap_CocoaDialog_InsertTable.nib \
	ap_CocoaDialog_Latex.nib \
	ap_CocoaDialog_ListRevisions.nib \
	ap_CocoaDialog_Lists.nib \
	ap_CocoaDialog_MailMerge.nib \
	ap_CocoaDialog_MarkRevisions.nib \
	ap_CocoaDialog_MergeCells.nib \
	ap_CocoaDialog_MetaData.nib \
	ap_CocoaDialog_New.nib \
	ap_CocoaDialog_Options.nib \
	ap_CocoaDialog_PageNumbers.nib \
	ap_CocoaDialog_PageSetup.nib \
	ap_CocoaDialog_Paragraph.nib \
	ap_CocoaDialog_Replace.nib \
	ap_CocoaDialog_Spell.nib \
	ap_CocoaDialog_SplitCells.nib \
	ap_CocoaDialog_Styles.nib \
	ap_CocoaDialog_StylesModify.nib \
	ap_CocoaDialog_Stylist.nib \
	ap_CocoaDialog_Tab.nib \
	ap_CocoaDialog_ToggleCase.nib \
	ap_CocoaDialog_WordCount.nib \
	ap_CocoaFrame.nib \
	$(NULL)
