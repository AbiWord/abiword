#! gmake

## AbiSource Application Framework
## Copyright (C) 1998,1999 AbiSource, Inc.
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

include $(ABI_ROOT)/src/config/abi_defs.mk

CPPSRCS=	xad_Document.cpp		\
		xap_App.cpp			\
		xap_AppImpl.cpp                  \
		xap_Args.cpp			\
		xap_Clipboard.cpp		\
		xap_Dialog.cpp			\
		xap_DialogFactory.cpp		\
		xap_Dictionary.cpp		\
		xap_Dlg_About.cpp		\
		xap_Dlg_ClipArt.cpp		\
		xap_Dlg_DocComparison.cpp	\
		xap_Dlg_FileOpenSaveAs.cpp	\
		xap_Dlg_FontChooser.cpp		\
		xap_Dlg_HTMLOptions.cpp		\
		xap_Dlg_MessageBox.cpp		\
		xap_Dlg_Password.cpp		\
		xap_Dlg_PluginManager.cpp	\
		xap_Dlg_Print.cpp		\
		xap_Dlg_PrintPreview.cpp        \
		xap_Dlg_WindowMore.cpp		\
		xap_Dlg_Zoom.cpp		\
		xap_Dlg_Insert_Symbol.cpp	\
		xap_Dlg_Image.cpp		\
		xap_FakeClipboard.cpp		\
		xap_FontPreview.cpp		\
		xap_Frame.cpp			\
		xap_FrameImpl.cpp		\
		xap_InputModes.cpp		\
		xap_LoadBindings.cpp		\
		xap_Log.cpp			\
		xap_Prefs.cpp			\
		xap_Preview.cpp			\
		xap_Preview_Zoom.cpp		\
		xap_Draw_Symbol.cpp		\
		xap_Scrollbar_ViewListener.cpp	\
		xap_StatusBar.cpp		\
		xap_Strings.cpp			\
		xap_Toolbar_ControlFactory.cpp	\
		xap_ViewListener.cpp		\
		xav_View.cpp			\
		xap_EncodingManager.cpp		\
		xap_Module.cpp			\
		xap_ModuleManager.cpp		\
		xap_Dlg_Language.cpp		\
		xap_Dlg_Encoding.cpp \
		xap_Dlg_ListDocuments.cpp \
		xap_Dlg_History.cpp \
		xap_Dlg_DocComparison.cpp


TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
