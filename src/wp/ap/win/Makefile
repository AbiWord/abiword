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
			ap_$(ABI_FE)AppImpl.cpp                 \
			ap_$(ABI_FE)Clipboard.cpp		\
			ap_$(ABI_FE)DialogFactory.cpp		\
			ap_$(ABI_FE)Dialog_New.cpp		\
			ap_$(ABI_FE)Dialog_Break.cpp		\
			ap_$(ABI_FE)Dialog_InsertTable.cpp	\
			ap_$(ABI_FE)Dialog_FormatTable.cpp	\
			ap_$(ABI_FE)Dialog_FormatFrame.cpp	\
			ap_$(ABI_FE)Dialog_Field.cpp		\
			ap_$(ABI_FE)Dialog_Insert_DateTime.cpp	\
			ap_$(ABI_FE)Dialog_Lists.cpp		\
			ap_$(ABI_FE)Dialog_Options.cpp		\
			ap_$(ABI_FE)Dialog_Replace.cpp		\
			ap_$(ABI_FE)Dialog_PageNumbers.cpp      \
			ap_$(ABI_FE)Dialog_Paragraph.cpp	\
			ap_$(ABI_FE)Dialog_Spell.cpp		\
			ap_$(ABI_FE)Dialog_Styles.cpp		\
			ap_$(ABI_FE)Dialog_Stylist.cpp		\
			ap_$(ABI_FE)Dialog_Goto.cpp		\
			ap_$(ABI_FE)Dialog_WordCount.cpp	\
			ap_$(ABI_FE)Dialog_Columns.cpp		\
			ap_$(ABI_FE)Dialog_Tab.cpp		\
			ap_$(ABI_FE)Dialog_PageSetup.cpp	\
			ap_$(ABI_FE)Dialog_ToggleCase.cpp	\
			ap_$(ABI_FE)Dialog_InsertBookmark.cpp	\
			ap_$(ABI_FE)Dialog_FormatTOC.cpp	\
			ap_$(ABI_FE)Dialog_FormatFootnotes.cpp	\
			ap_$(ABI_FE)Dialog_InsertHyperlink.cpp	\
			ap_$(ABI_FE)Dialog_MetaData.cpp		\
			ap_$(ABI_FE)Dialog_MailMerge.cpp	\
			ap_$(ABI_FE)Dialog_Latex.cpp	\
			ap_$(ABI_FE)Frame.cpp			\
			ap_$(ABI_FE)FrameImpl.cpp			\
			ap_$(ABI_FE)LeftRuler.cpp		\
			ap_$(ABI_FE)Prefs.cpp			\
			ap_$(ABI_FE)StatusBar.cpp		\
			ap_$(ABI_FE)Toolbar_StyleCombo.cpp	\
			ap_$(ABI_FE)Toolbar_FontCombo.cpp	\
			ap_$(ABI_FE)Toolbar_SizeCombo.cpp	\
			ap_$(ABI_FE)Toolbar_ZoomCombo.cpp	\
			ap_$(ABI_FE)_TB_CFactory.cpp		\
			ap_$(ABI_FE)Dialog_Background.cpp	\
			ap_$(ABI_FE)Dialog_HdrFtr.cpp	\
			ap_$(ABI_FE)Dialog_MarkRevisions.cpp	\
			ap_$(ABI_FE)Dialog_ListRevisions.cpp	\
			ap_$(ABI_FE)Dialog_MergeCells.cpp	\
			ap_$(ABI_FE)Dialog_SplitCells.cpp	\
			ap_$(ABI_FE)TopRuler.cpp

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

INCLUDES+= -I$(ABI_XX_ROOT)/../wv -I$(ABI_XX_ROOT)/../wv/exporter -I$(ABI_ZLIB_INC) -I$(ABI_XX_ROOT)/../wv/libole2 -I$(ABI_XX_ROOT)/../libpng -I$(ABI_XX_ROOT)/../libiconv/include

build:: $(TARGETS)
