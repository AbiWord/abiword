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

ABI_WP_INCS+=	/wp/ap/xp	/wp/ap/$(ABI_NATIVE)

CPPSRCS=	ap_Dialog_Replace.cpp		\
		ap_EditMethods.cpp		\
		ap_Frame.cpp			\
		ap_FrameData.cpp		\
		ap_LoadBindings.cpp		\
		ap_LoadBindings_DeadAbovedot.cpp	\
		ap_LoadBindings_DeadAcute.cpp		\
		ap_LoadBindings_DeadBreve.cpp		\
		ap_LoadBindings_DeadCaron.cpp		\
		ap_LoadBindings_DeadCedilla.cpp		\
		ap_LoadBindings_DeadCircumflex.cpp	\
		ap_LoadBindings_DeadDiaeresis.cpp	\
		ap_LoadBindings_DeadDoubleacute.cpp	\
		ap_LoadBindings_DeadGrave.cpp		\
		ap_LoadBindings_DeadMacron.cpp		\
		ap_LoadBindings_DeadOgonek.cpp		\
		ap_LoadBindings_DeadTilde.cpp		\
		ap_LoadBindings_Default.cpp	\
		ap_Menu_ActionSet.cpp		\
		ap_Menu_Functions.cpp		\
		ap_Menu_LabelSet.cpp		\
		ap_Menu_Layouts.cpp		\
		ap_Toolbar_ActionSet.cpp	\
		ap_Toolbar_Functions.cpp	\
		ap_Toolbar_Icons.cpp		\
		ap_Toolbar_LabelSet.cpp		\
		ap_Toolbar_Layouts.cpp

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

build:: $(TARGETS)
