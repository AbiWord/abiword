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

include $(ABI_ROOT)/src/config/abi_defs_wp.mk

ABI_WP_INCS+=	/wp/ap/xp	/wp/ap/$(ABI_NATIVE)

CPPSRCS=	ap_Dialog_Break.cpp		\
		ap_Dialog_Replace.cpp		\
		ap_Dialog_Goto.cpp		\
		ap_Dialog_Spell.cpp		\
		ap_EditMethods.cpp		\
		ap_FrameData.cpp		\
		ap_LeftRuler.cpp		\
		ap_LoadBindings.cpp		\
		ap_LB_DeadAbovedot.cpp		\
		ap_LB_DeadAcute.cpp		\
		ap_LB_DeadBreve.cpp		\
		ap_LB_DeadCaron.cpp		\
		ap_LB_DeadCedilla.cpp		\
		ap_LB_DeadCircumflex.cpp	\
		ap_LB_DeadDiaeresis.cpp		\
		ap_LB_DeadDoubleacute.cpp	\
		ap_LB_DeadGrave.cpp		\
		ap_LB_DeadMacron.cpp		\
		ap_LB_DeadOgonek.cpp		\
		ap_LB_DeadTilde.cpp		\
		ap_LB_Default.cpp		\
		ap_LB_Emacs.cpp			\
		ap_LB_EmacsCtrlX.cpp		\
		ap_LB_viEdit.cpp		\
		ap_LB_viEdit_colon.cpp		\
		ap_LB_viEdit_c.cpp		\
		ap_LB_viEdit_d.cpp		\
		ap_LB_viEdit_y.cpp		\
		ap_LB_viInput.cpp		\
		ap_Menu_ActionSet.cpp		\
		ap_Menu_Functions.cpp		\
		ap_Menu_LabelSet.cpp		\
		ap_Menu_Layouts.cpp		\
		ap_Prefs.cpp			\
		ap_Ruler.cpp			\
		ap_StatusBar.cpp		\
		ap_Strings.cpp			\
		ap_Toolbar_ActionSet.cpp	\
		ap_Toolbar_Functions.cpp	\
		ap_Toolbar_Icons.cpp		\
		ap_Toolbar_LabelSet.cpp		\
		ap_Toolbar_Layouts.cpp		\
		ap_TopRuler.cpp

GENCPPSRCS=	$(OBJDIR)/ap_wp_splash.cpp	\
		$(OBJDIR)/ap_wp_sidebar.cpp

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)

ifdef ABISOURCE_LICENSED_TRADEMARKS

$(OBJDIR)/ap_wp_splash.cpp: splash_tm.png $(BINDIR)/cdump$(EXE_SUFFIX)
	rm -f $(OBJDIR)/ap_wp_splash.cpp
	$(BINDIR)/cdump$(EXE_SUFFIX) splash_tm.png g_pngSplash > $(OBJDIR)/ap_wp_splash.cpp

$(OBJDIR)/ap_wp_sidebar.cpp: sidebar_tm.png $(BINDIR)/cdump$(EXE_SUFFIX)
	rm -f $(OBJDIR)/ap_wp_sidebar.cpp
	$(BINDIR)/cdump$(EXE_SUFFIX) sidebar_tm.png g_pngSidebar > $(OBJDIR)/ap_wp_sidebar.cpp

else

$(OBJDIR)/ap_wp_splash.cpp: splash.png $(BINDIR)/cdump$(EXE_SUFFIX)
	rm -f $(OBJDIR)/ap_wp_splash.cpp
	$(BINDIR)/cdump$(EXE_SUFFIX) ./splash.png g_pngSplash > $(OBJDIR)/ap_wp_splash.cpp

$(OBJDIR)/ap_wp_sidebar.cpp: sidebar.png $(BINDIR)/cdump$(EXE_SUFFIX)
	rm -f $(OBJDIR)/ap_wp_sidebar.cpp
	$(BINDIR)/cdump$(EXE_SUFFIX) ./sidebar.png g_pngSidebar > $(OBJDIR)/ap_wp_sidebar.cpp

endif

