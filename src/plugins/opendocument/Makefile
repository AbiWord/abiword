#! gmake

## AbiWord
## Copyright (C) 2005 Marc Maurer <uwog@abisource.com>
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

ABI_ROOT:=$(shell pwd)/../../../../abi
include $(ABI_ROOT)/src/config/abi_defs.mk

DIRS=				common exp imp	

LIBRARY_NAME=		AbiOpenDocument
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=			$(PLUGIN)

OBJS=				$(OBJDIR)/ie_exp_OpenDocument.$(OBJ_SUFFIX) \
				$(OBJDIR)/ie_imp_OpenDocument.$(OBJ_SUFFIX) \
				$(OBJDIR)/ie_impexp_OpenDocument.$(OBJ_SUFFIX) \
				$(OBJDIR)/ie_exp_OpenDocument_Sniffer.$(OBJ_SUFFIX) \
				$(OBJDIR)/ie_imp_OpenDocument_Sniffer.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Abi_Data.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_ContentStream_ListenerState.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_FontFaceDecls.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_ElementStack.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Frame_ListenerState.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_ListenerStateAction.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_ListLevelStyle.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_ManifestStream_ListenerState.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_MetaStream_ListenerState.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_NotesConfiguration.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Office_Styles.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Postpone_ListenerState.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_StartTag.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_StreamListener.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Style_List.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Style_MasterPage.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Style_PageLayout.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_StylesStream_ListenerState.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Style_Style.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Style_Style_Family.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_Table_ListenerState.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_TableOfContent_ListenerState.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_TextContent_ListenerState.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODi_XMLRecorder.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_AbiDocListener.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_AbiDocListenerImpl.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_AutomaticStyles.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_AuxiliaryData.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Common.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_DocumentData.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_FontFaceDecls.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Frame_Listener.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_HeadingSearcher_Listener.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_ListLevelStyle.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Main_Listener.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_ManifestWriter.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_MetaDataWriter.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Note_Listener.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_PicturesWriter.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_SettingsWriter.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Style_List.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Style_MasterPage.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Style_PageLayout.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Style_Style.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Styles.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Table_Listener.$(OBJ_SUFFIX) \
				$(OBJDIR)/ODe_Text_Listener.$(OBJ_SUFFIX)

ifeq ($(ABI_FE), Win32)
ifeq ($(OS_NAME), MINGW32)
	EXTRA_LIBS+=  -lAbiWord.dll
	EXTRA_LIBS+=  $(shell pkg-config --libs libgsf-1)
endif
endif

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
