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

CPPSRCS=	ad_Document.cpp				\
		ap_App.cpp				\
		ap_Args.cpp				\
		ap_Dialog.cpp				\
		ap_DialogFactory.cpp			\
		ap_Dialog_FileOpenSaveAs.cpp		\
		ap_Dialog_MessageBox.cpp		\
		ap_Dialog_Print.cpp			\
		ap_Frame.cpp				\
		ap_ViewListener.cpp			\
		av_View.cpp

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

build:: $(TARGETS)
