#! gmake

## AbiWord
## Copyright (C) 1998-2000 AbiSource, Inc.
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

include $(ABI_ROOT)/src/config/abi_defs_wp.mk

ABI_WP_INCS+=	/wp/ap/xp	/wp/ap/$(ABI_NATIVE)

CPPSRCS=		ap_$(ABI_FE)App.cpp			\
			ap_$(ABI_FE)Clipboard.cpp		\
			ap_$(ABI_FE)DialogFactory.cpp		\
			ap_$(ABI_FE)Dialog_Break.cpp		\
			ap_$(ABI_FE)Dialog_Field.cpp		\
			ap_$(ABI_FE)Dialog_Insert_DateTime.cpp	\
			ap_$(ABI_FE)Dialog_Lists.cpp		\
			ap_$(ABI_FE)Dialog_Options.cpp		\
			ap_$(ABI_FE)Dialog_Replace.cpp		\
			ap_$(ABI_FE)Dialog_PageNumbers.cpp      \
			ap_$(ABI_FE)Dialog_Paragraph.cpp	\
			ap_$(ABI_FE)Dialog_Spell.cpp		\
			ap_$(ABI_FE)Dialog_Styles.cpp		\
			ap_$(ABI_FE)Dialog_Stub.cpp		\
			ap_$(ABI_FE)Dialog_Goto.cpp		\
			ap_$(ABI_FE)Dialog_WordCount.cpp	\
			ap_$(ABI_FE)Dialog_Columns.cpp		\
			ap_$(ABI_FE)Dialog_Tab.cpp		\
			ap_$(ABI_FE)Dialog_PageSetup.cpp	\
			ap_$(ABI_FE)Dialog_ToggleCase.cpp	\
			ap_$(ABI_FE)Frame.cpp			\
			ap_$(ABI_FE)LeftRuler.cpp		\
			ap_$(ABI_FE)Prefs.cpp			\
			ap_$(ABI_FE)StatusBar.cpp		\
			ap_$(ABI_FE)Toolbar_StyleCombo.cpp	\
			ap_$(ABI_FE)Toolbar_FontCombo.cpp	\
			ap_$(ABI_FE)Toolbar_SizeCombo.cpp	\
			ap_$(ABI_FE)Toolbar_ZoomCombo.cpp	\
			ap_$(ABI_FE)_TB_CFactory.cpp		\
			ap_$(ABI_FE)Dialog_Background.cpp	\
			ap_$(ABI_FE)TopRuler.cpp

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

# Win32 must build libiconv as a peer, Unix should either
# have iconv.h available as part of its C library (GNU libc
# systems) or have it installed as part of the libiconv-1.0
# package.

# Test for iconv in system locations
HAVE_ICONV_SYSTEM := $(shell if [ -r /usr/include/iconv.h -o -r /usr/local/include/iconv.h ] ; then echo 1 ; fi)

ifeq ($(OS_NAME), WIN32)
INCLUDES=		-I$(ABI_XX_ROOT)/../libiconv/include
else
ifneq ($(HAVE_ICONV_SYSTEM),1)
INCLUDES= 		-I$(ABI_ROOT)/../libiconv/include
endif
endif

build:: $(TARGETS)
