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

ABI_ROOT:=$(shell pwd)/../../../../../../abi
include $(ABI_ROOT)/src/config/abi_defs.mk

TARGETS=			$(OBJS)
CPPSRCS=			ODe_AbiDocListener.cpp \
				ODe_AbiDocListenerImpl.cpp \
				ODe_AutomaticStyles.cpp \
				ODe_AuxiliaryData.cpp \
				ODe_Common.cpp \
				ODe_DocumentData.cpp \
				ODe_FontFaceDecls.cpp \
				ODe_Frame_Listener.cpp \
				ODe_HeadingSearcher_Listener.cpp \
				ODe_ListLevelStyle.cpp \
				ODe_Main_Listener.cpp \
				ODe_ManifestWriter.cpp \
				ODe_MetaDataWriter.cpp \
				ODe_Note_Listener.cpp \
				ODe_PicturesWriter.cpp \
				ODe_SettingsWriter.cpp \
				ODe_Style_List.cpp \
				ODe_Style_MasterPage.cpp \
				ODe_Style_PageLayout.cpp \
				ODe_Style_Style.cpp \
				ODe_Styles.cpp \
				ODe_Table_Listener.cpp \
				ODe_Text_Listener.cpp \
				ie_exp_OpenDocument.cpp \
				ie_exp_OpenDocument_Sniffer.cpp

ifeq ($(ABI_FE), Win32)
ifeq ($(OS_NAME), MINGW32)
	INCLUDES+=  $(shell pkg-config --cflags libgsf-1)
endif
endif

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
