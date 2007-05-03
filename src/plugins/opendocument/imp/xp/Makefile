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
CPPSRCS=			ie_imp_OpenDocument.cpp \
				ie_imp_OpenDocument_Sniffer.cpp \
				ODi_Abi_Data.cpp \
				ODi_ContentStream_ListenerState.cpp \
				ODi_FontFaceDecls.cpp \
				ODi_ElementStack.cpp \
				ODi_Frame_ListenerState.cpp \
				ODi_ListenerStateAction.cpp \
				ODi_ListLevelStyle.cpp \
				ODi_ManifestStream_ListenerState.cpp \
				ODi_MetaStream_ListenerState.cpp \
				ODi_NotesConfiguration.cpp \
				ODi_Office_Styles.cpp \
				ODi_Postpone_ListenerState.cpp \
				ODi_StartTag.cpp \
				ODi_StreamListener.cpp \
				ODi_Style_List.cpp \
				ODi_Style_MasterPage.cpp \
				ODi_Style_PageLayout.cpp \
				ODi_StylesStream_ListenerState.cpp \
				ODi_Style_Style.cpp \
				ODi_Style_Style_Family.cpp \
				ODi_Table_ListenerState.cpp \
				ODi_TableOfContent_ListenerState.cpp \
				ODi_TextContent_ListenerState.cpp \
				ODi_XMLRecorder.cpp

ifeq ($(ABI_FE), Win32)
ifeq ($(OS_NAME), MINGW32)
	INCLUDES+=  $(shell pkg-config --cflags libgsf-1)
endif
endif

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
