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

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs_hello.mk

CPPSRCS=	ap_EditMethods.cpp		\
		ap_FrameData.cpp 		\
		ap_LB_Default.cpp		\
		ap_LoadBindings.cpp		\
		ap_Menu_ActionSet.cpp		\
		ap_Menu_Functions.cpp		\
		ap_Menu_LabelSet.cpp		\
		ap_Menu_Layouts.cpp		\
		ap_Prefs.cpp 			\
		ap_StatusBar.cpp		\
		ap_Strings.cpp			\
		ap_Toolbar_ActionSet.cpp	\
		ap_Toolbar_Icons.cpp 		\
		ap_Toolbar_LabelSet.cpp		\
		ap_Toolbar_Layouts.cpp		\
		ap_View.cpp \
		ap_hello_sidebar.cpp

#GENCPPSRCS=	$(OBJDIR)/ap_hello_sidebar.cpp

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)


#$(OBJDIR)/ap_hello_sidebar.cpp: sidebar.png $(BINDIR)/cdump$(EXE_SUFFIX)
#	rm -f $(OBJDIR)/ap_hello_sidebar.cpp
#	$(BINDIR)/cdump$(EXE_SUFFIX) ./sidebar.png g_pngSidebar > $(OBJDIR)/ap_hello_sidebar.cpp
