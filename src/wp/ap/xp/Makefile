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

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

EXPORT_HEADERS=		ap_App.h			\
			ap_EditMethods.h		\
			ap_Frame.h			\
			ap_LoadBindings.h		\
			ap_Menu_ActionSet.h		\
			ap_Menu_Functions.h		\
			ap_Menu_LabelSet.h		\
			ap_Menu_Layouts.h		\
			ap_Menu_Id.h

HEADERS=		$(EXPORT_HEADERS)		\
			ap_LoadBindings_Default.h	\
			ap_Menu_LabelSet_EnUS.h		\
			ap_Menu_LabelSet_Languages.h	\
			ap_Menu_Layouts.h		\
			ap_Menu_Layouts_All.h		\
			ap_Menu_Layouts_DebugMenu.h	\
			ap_Menu_Layouts_MainMenu.h \
			ap_ViewListener.h

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		ap_App.cpp			\
			ap_EditMethods.cpp		\
			ap_Frame.cpp			\
			ap_LoadBindings.cpp		\
			ap_LoadBindings_Default.cpp	\
			ap_Menu_ActionSet.cpp		\
			ap_Menu_Functions.cpp		\
			ap_Menu_LabelSet.cpp		\
			ap_Menu_Layouts.cpp		\
			ap_ViewListener.cpp

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

build:: $(TARGETS)
