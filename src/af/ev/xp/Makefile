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

ABI_DEPTH = ../..

include $(ABI_DEPTH)/config/abi_defs.mk

## TODO: decide if all of these need to be exported.
## TODO: move the ones which dont to the HEADERS definition

EXPORT_HEADERS=		ev_EditBinding.h	\
			ev_EditBits.h		\
			ev_EditEventMapper.h	\
			ev_EditMethod.h		\
			ev_Keyboard.h		\
			ev_Menu.h		\
			ev_Menu_Actions.h	\
			ev_Menu_Labels.h	\
			ev_Menu_Layouts.h	\
			ev_Mouse.h		\
			ev_NamedVirtualKey.h	

HEADERS=		$(EXPORT_HEADERS)

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		ev_EditBinding.cpp		\
			ev_EditEventMapper.cpp		\
			ev_EditMethod.cpp		\
			ev_Keyboard.cpp			\
			ev_Menu.cpp			\
			ev_Menu_Actions.cpp		\
			ev_Menu_Labels.cpp		\
			ev_Menu_Layouts.cpp		\
			ev_Mouse.cpp			\
			ev_NamedVirtualKey.cpp		


INCLUDES=		-I$(DIST)/include -I. -DUT_DEBUG

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

export:: $(EXPORT_HEADERS)
	@echo Exporting headers from `pwd`...
	@$(subst xxxx,$(DIST)/include,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(EXPORT_HEADERS) $(DIST)/include

libs:: $(TARGETS)
