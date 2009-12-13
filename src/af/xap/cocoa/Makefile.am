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

# TODO: fix the wp include, needed because of ap_Features.h
AM_CPPFLAGS = \
	$(IMPEXP_CPPFLAGS) \
	-x objective-c++ \
	-I$(top_srcdir)/src/wp/ap/xp \
	-I$(top_srcdir)/src/wp/ap/cocoa

#	-DDATADIR="\"$(datadir)\""

nobase_resources_DATA = xap_CocoaDlg_About.nib/*.nib \
	xap_CocoaDlg_DocComparison.nib/*.nib \
	xap_CocoaDlg_FontChooser.nib/*.nib \
	xap_CocoaDlg_HTMLOptions.nib/*.nib \
	xap_CocoaDlg_History.nib/*.nib \
	xap_CocoaDlg_Image.nib/*.nib \
	xap_CocoaDlg_Insert_Symbol.nib/*.nib \
	xap_CocoaDlg_Language.nib/*.nib \
	xap_CocoaDlg_Password.nib/*.nib \
	xap_CocoaDlg_PluginManager.nib/*.nib \
	xap_CocoaDlg_Zoom.nib/*.nib \
	xap_CocoaFileOpen_Views.nib/*.nib \
	xap_CocoaToolPalette.nib/*.nib \
	xap_GenericListChooser_Controller.nib/*.nib \
	$(NULL)

libcocoa_la_SOURCES = \
	xap_CocoaAbiConversions.h \
	xap_CocoaAbiConversions.cpp \
	xap_CocoaAppController.h \
	xap_CocoaAppController.cpp \
	xap_CocoaApp.h \
	xap_CocoaAppImpl.h \
	xap_CocoaAppImpl.cpp \
	xap_CocoaApp.cpp \
	xap_CocoaAssert.h \
	xap_CocoaAssert.cpp \
	xap_CocoaClipboard.h \
	xap_CocoaClipboard.cpp \
	xap_CocoaDialogFactory.h \
	xap_CocoaDialog_Utilities.h \
	xap_CocoaDialog_Utilities.cpp \
	xap_CocoaDlg_About.h \
	xap_CocoaDlg_About.cpp \
	xap_CocoaDlg_ClipArt.h \
	xap_CocoaDlg_ClipArt.cpp \
	xap_CocoaDlg_DocComparison.h \
	xap_CocoaDlg_DocComparison.cpp \
	xap_CocoaDlg_Encoding.h \
	xap_CocoaDlg_Encoding.cpp \
	xap_CocoaDlg_FileOpenSaveAs.h \
	xap_CocoaDlg_FileOpenSaveAs.cpp \
	xap_CocoaDlg_FontChooser.h \
	xap_CocoaDlg_FontChooser.cpp \
	xap_CocoaDlg_History.h \
	xap_CocoaDlg_History.cpp \
	xap_CocoaDlg_HTMLOptions.h \
	xap_CocoaDlg_HTMLOptions.cpp \
	xap_CocoaDlg_Image.h \
	xap_CocoaDlg_Image.cpp \
	xap_CocoaDlg_Insert_Symbol.h \
	xap_CocoaDlg_Insert_Symbol.cpp \
	xap_CocoaDlg_Language.h \
	xap_CocoaDlg_Language.cpp \
	xap_CocoaDlg_ListDocuments.h \
	xap_CocoaDlg_ListDocuments.cpp \
	xap_CocoaDlg_MessageBox.h \
	xap_CocoaDlg_MessageBox.cpp \
	xap_CocoaDlg_Password.h \
	xap_CocoaDlg_Password.cpp \
	xap_CocoaDlg_PluginManager.h \
	xap_CocoaDlg_PluginManager.cpp \
	xap_CocoaDlg_Print.h \
	xap_CocoaDlg_Print.cpp \
	xap_CocoaDlg_PrintPreview.h \
	xap_CocoaDlg_PrintPreview.cpp \
	xap_CocoaDlg_WindowMore.h \
	xap_CocoaDlg_WindowMore.cpp \
	xap_CocoaDlg_Zoom.h \
	xap_CocoaDlg_Zoom.cpp \
	xap_CocoaEncodingManager.h \
	xap_CocoaEncodingManager.cpp \
	xap_CocoaFeatures.h \
	xap_CocoaFrame.h \
	xap_CocoaFrameImpl.h \
	xap_CocoaFrameImpl.cpp \
	xap_CocoaFrame.cpp \
	xap_CocoaModule.h \
	xap_CocoaModule.cpp \
	xap_Cocoa_NSTableUtils.h \
	xap_Cocoa_NSTableUtils.cpp \
	xap_CocoaPlugin.h \
	xap_CocoaPlugin.cpp \
	xap_Cocoa_ResourceIDs.h \
	xap_Cocoa_ResourceIDs.cpp \
	xap_Cocoa_TB_CFactory.h \
	xap_CocoaTextView.h \
	xap_CocoaTextView.cpp \
	xap_CocoaTimer.h \
	xap_CocoaTimer.cpp \
	xap_CocoaToolbar_Icons.h \
	xap_CocoaToolbar_Icons.cpp \
	xap_CocoaToolbarWindow.h \
	xap_CocoaToolbarWindow.cpp \
	xap_CocoaToolPalette.h \
	xap_CocoaToolPalette.cpp \
	xap_CocoaToolProvider.h \
	xap_CocoaToolProvider.cpp \
	xap_CocoaTools.h \
	xap_CocoaWidget.h \
	xap_CocoaWidget.cpp \
	xap_CocoaWindow.h \
	xap_CocoaWindow.cpp \
	xap_FrameNSWindow.h \
	xap_FrameNSWindow.cpp \
	xap_GenericListChooser_Controller.h \
	xap_GenericListChooser_Controller.cpp \
	xap_PrintingDelegate.h \
	xap_PrintingDelegate.cpp \
	xap_PrintingNSView.h \
	xap_PrintingNSView.cpp

EXTRA_DIST = xap_CocoaDlg_About.nib \
	xap_CocoaDlg_DocComparison.nib \
	xap_CocoaDlg_FontChooser.nib \
	xap_CocoaDlg_HTMLOptions.nib \
	xap_CocoaDlg_History.nib \
	xap_CocoaDlg_Image.nib \
	xap_CocoaDlg_Insert_Symbol.nib \
	xap_CocoaDlg_Language.nib \
	xap_CocoaDlg_Password.nib \
	xap_CocoaDlg_PluginManager.nib \
	xap_CocoaDlg_Zoom.nib \
	xap_CocoaFileOpen_Views.nib \
	xap_CocoaToolPalette.nib \
	xap_GenericListChooser_Controller.nib \
	$(NULL)
