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

ABI_ROOT:=$(shell pwd)/../../../../..

include $(ABI_ROOT)/src/config/abi_defs_wp.mk

ABI_WP_INCS+=	/wp/ap/$(ABI_NATIVE)/$(ABI_GNOME_DIR)

PLATFORM_OBJS+=	$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)App.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Break.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Replace.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_WordCount.$(OBJ_SUFFIX)
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)DialogFactory.$(OBJ_SUFFIX)
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Clipboard.$(OBJ_SUFFIX)		\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Paragraph.$(OBJ_SUFFIX)		\

#       	$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Spell.$(OBJ_SUFFIX)		\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Frame.$(OBJ_SUFFIX)			\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)LeftRuler.$(OBJ_SUFFIX)		\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Prefs.$(OBJ_SUFFIX)			\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)StatusBar.$(OBJ_SUFFIX)		\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Toolbar_StyleCombo.$(OBJ_SUFFIX)	\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Toolbar_FontCombo.$(OBJ_SUFFIX)	\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Toolbar_SizeCombo.$(OBJ_SUFFIX)	\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Toolbar_ZoomCombo.$(OBJ_SUFFIX)	\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)_TB_CFactory.$(OBJ_SUFFIX)		\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)TopRuler.$(OBJ_SUFFIX)			\
#		$(OBJDIR)/ap_$(ABI_FE)$(ABI_GNOME_PREFIX)ViewListener.$(OBJ_SUFFIX)

CPPSRCS=		ap_$(ABI_FE)$(ABI_GNOME_PREFIX)App.cpp			\
			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Break.cpp		\
			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Replace.cpp	\
			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_WordCount.cpp
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)DialogFactory.cpp
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Clipboard.cpp		\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Paragraph.cpp	\

#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dialog_Spell.cpp		\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Frame.cpp		\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)LeftRuler.cpp		\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Prefs.cpp		\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)StatusBar.cpp		\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Toolbar_StyleCombo.cpp	\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Toolbar_FontCombo.cpp	\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Toolbar_SizeCombo.cpp	\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)Toolbar_ZoomCombo.cpp	\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)_TB_CFactory.cpp		\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)TopRuler.cpp		\
#			ap_$(ABI_FE)$(ABI_GNOME_PREFIX)ViewListener.cpp

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
