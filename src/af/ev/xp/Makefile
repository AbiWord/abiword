#! gmake

## AbiSource Program Utilities
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

include $(ABI_ROOT)/src/config/abi_defs.mk

CPPSRCS=		ev_EditBinding.cpp		\
			ev_EditEventMapper.cpp		\
			ev_EditMethod.cpp		\
			ev_Keyboard.cpp			\
			ev_Menu.cpp			\
			ev_Menu_Actions.cpp		\
			ev_Menu_Labels.cpp		\
			ev_Menu_Layouts.cpp		\
			ev_Mouse.cpp			\
			ev_NamedVirtualKey.cpp		\
			ev_Toolbar.cpp			\
			ev_Toolbar_Actions.cpp		\
			ev_Toolbar_Control.cpp		\
			ev_Toolbar_Labels.cpp		\
			ev_Toolbar_Layouts.cpp

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
