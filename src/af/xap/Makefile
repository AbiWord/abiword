#! gmake

## AbiWord
## Copyright (C) 1998,1999 AbiSource, Inc.
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

ABI_ROOT:=$(shell pwd)/../../..

include $(ABI_ROOT)/src/config/abi_defs.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiXAp
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

PLATFORM_OBJS=	$(OBJDIR)/xap_$(ABI_FE)App.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_$(ABI_FE)Clipboard.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_$(ABI_FE)Dlg_About.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_$(ABI_FE)Dlg_FileOpenSaveAs.$(OBJ_SUFFIX)		\
		$(OBJDIR)/xap_$(ABI_FE)Dlg_FontChooser.$(OBJ_SUFFIX)		\
		$(OBJDIR)/xap_$(ABI_FE)Dlg_MessageBox.$(OBJ_SUFFIX)		\
		$(OBJDIR)/xap_$(ABI_FE)Dlg_Print.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_$(ABI_FE)Dlg_PrintPreview.$(OBJ_SUFFIX)           \
		$(OBJDIR)/xap_$(ABI_FE)Dlg_WindowMore.$(OBJ_SUFFIX)		\
		$(OBJDIR)/xap_$(ABI_FE)Dlg_Zoom.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_$(ABI_FE)Dlg_Insert_Symbol.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_$(ABI_FE)Frame.$(OBJ_SUFFIX)

ifeq ($(ABI_FE), Mac)
PLATFORM_OBJS += $(OBJDIR)/xap_$(ABI_FE)Tlbr_Icons.$(OBJ_SUFFIX)
else
PLATFORM_OBJS += $(OBJDIR)/xap_$(ABI_FE)Toolbar_Icons.$(OBJ_SUFFIX)
endif

# Note that we might later want to include the FakeClipboard code only for certain platforms.
ifeq ($(ABI_FE), BeOS)
PLATFORM_OBJS += $(OBJDIR)/be_GRDrawView.$(OBJ_SUFFIX)
endif

# THESE ARE OBJECTS EXCLUSIVE TO THE UNIX FRONT-END
ifeq ($(ABI_FE), Unix)
PLATFORM_OBJS += 	$(OBJDIR)/xap_$(ABI_FE)Font.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)FontManager.$(OBJ_SUFFIX)	\
			$(OBJDIR)/xap_$(ABI_FE)FontXLFD.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)PSFont.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)PSGenerate.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)PSParseAFM.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)PSImage.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)PSGraphics.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)EncodingManager.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_FE), Win32)
PLATFORM_OBJS +=	$(OBJDIR)/xap_$(ABI_FE)Slurp.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)PreviewWidget.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)DialogHelper.$(OBJ_SUFFIX)
endif

ifdef ABI_OPT_GNOME
PLATFORM_OBJS +=	$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)App.$(OBJ_SUFFIX)		\
			$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dlg_About.$(OBJ_SUFFIX)	\
			$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dlg_FontChooser.$(OBJ_SUFFIX)	\
			$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dlg_Insert_Symbol.$(OBJ_SUFFIX)	\
			$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dlg_MessageBox.$(OBJ_SUFFIX)	\
			$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dlg_Print.$(OBJ_SUFFIX) 	\
			$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dlg_PrintPreview.$(OBJ_SUFFIX) \
			$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)Dlg_Zoom.$(OBJ_SUFFIX)	\
			$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)Frame.$(OBJ_SUFFIX)           \
			$(OBJDIR)/xap_$(ABI_FE)$(ABI_GNOME_PREFIX)PrintGraphics.$(OBJ_SUFFIX)
endif

OBJS=		$(PLATFORM_OBJS)						\
		$(OBJDIR)/xad_Document.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_App.$(OBJ_SUFFIX)					\
		$(OBJDIR)/xap_Args.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_Clipboard.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_Dialog.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_DialogFactory.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_Dictionary.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_Dlg_About.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_Dlg_FileOpenSaveAs.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_Dlg_FontChooser.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_Dlg_MessageBox.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_Dlg_Print.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_Dlg_PrintPreview.$(OBJ_SUFFIX)                    \
		$(OBJDIR)/xap_Dlg_WindowMore.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_Dlg_Zoom.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_Dlg_Insert_Symbol.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_FakeClipboard.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_Frame.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_LoadBindings.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_Prefs.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_Preview.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_Preview_Zoom.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_Draw_Symbol.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xap_Scrollbar_ViewListener.$(OBJ_SUFFIX)		\
		$(OBJDIR)/xap_Strings.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_Toolbar_ControlFactory.$(OBJ_SUFFIX)		\
		$(OBJDIR)/xap_ViewListener.$(OBJ_SUFFIX)			\
		$(OBJDIR)/xav_View.$(OBJ_SUFFIX)				\
		$(OBJDIR)/xap_EncodingManager.$(OBJ_SUFFIX)

include $(ABI_ROOT)/src/config/abi_rules.mk


build:: $(TARGETS)
