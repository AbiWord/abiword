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

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs.mk

CPPSRCS=		xap_Win32App.cpp			\
			xap_Win32Clipboard.cpp			\
			xap_Win32Dlg_About.cpp			\
			xap_Win32Dlg_FileOpenSaveAs.cpp		\
			xap_Win32Dlg_FontChooser.cpp		\
			xap_Win32Dlg_MessageBox.cpp		\
			xap_Win32Dlg_Print.cpp			\
			xap_Win32Dlg_WindowMore.cpp		\
			xap_Win32Dlg_Zoom.cpp			\
			xap_Win32Frame.cpp			\
			xap_Win32Slurp.cpp			\
			xap_Win32Toolbar_Icons.cpp

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
