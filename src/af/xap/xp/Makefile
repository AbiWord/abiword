#! gmake

## AbiSource Application Framework
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

ABI_DEPTH = ../..

include $(ABI_DEPTH)/config/abi_defs.mk

CPPSRCS=	xad_Document.cpp		\
		xap_App.cpp			\
		xap_Args.cpp			\
		xap_Clipboard.cpp		\
		xap_Dialog.cpp			\
		xap_DialogFactory.cpp		\
		xap_Dialog_FileOpenSaveAs.cpp	\
		xap_Dialog_FontChooser.cpp	\
		xap_Dialog_MessageBox.cpp	\
		xap_Dialog_Print.cpp		\
		xap_FakeClipboard.cpp		\
		xap_Frame.cpp			\
		xap_Scrollbar_ViewListener.cpp	\
		xap_Toolbar_ControlFactory.cpp	\
		xap_ViewListener.cpp		\
		xav_View.cpp

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

build:: $(TARGETS)
