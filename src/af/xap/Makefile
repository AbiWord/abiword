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

ABI_DEPTH=		../..

include $(ABI_DEPTH)/config/abi_defs.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiXAp
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

# note: we assume that this will be used as a static library
# note: only.  if we want to use it as a shared library, then
# note: we probably need to append $(SHARED_LIBRARY) to TARGETS
# note: and add a line to the install section which copies it to
# note: the $(DIST)/bin directory on Windows.

PLATFORM_OBJS=	$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)App.$(OBJ_SUFFIX)				\
		$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Dlg_About.$(OBJ_SUFFIX)			\
		$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Dlg_FileOpenSaveAs.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Dlg_FontChooser.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Dlg_MessageBox.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Dlg_Print.$(OBJ_SUFFIX)			\
		$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Dlg_WindowMore.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Dlg_Zoom.$(OBJ_SUFFIX)			\
		$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Frame.$(OBJ_SUFFIX)			\
		$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Toolbar_Icons.$(OBJ_SUFFIX)

# Note that we might later want to include the FakeClipboard code only for certain platforms.
ifeq ($(ABI_FE), BeOS)
PLATFORM_OBJS += $(ABI_NATIVE)/$(OBJDIR)/be_GRDrawView.$(OBJ_SUFFIX)
endif

# THESE ARE OBJECTS EXCLUSIVE TO THE UNIX FRONT-END
ifeq ($(ABI_FE), Unix)
PLATFORM_OBJS += 	$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Font.$(OBJ_SUFFIX)			\
			$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)FontManager.$(OBJ_SUFFIX)		\
			$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)FontXLFD.$(OBJ_SUFFIX)		\
			$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)PSFont.$(OBJ_SUFFIX)		\
			$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)PSGenerate.$(OBJ_SUFFIX)		\
			$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)PSParseAFM.$(OBJ_SUFFIX)		\
			$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)PSImage.$(OBJ_SUFFIX)		\
			$(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)PSGraphics.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_FE), Win32)
PLATFORM_OBJS += $(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Clipboard.$(OBJ_SUFFIX)	\
		 $(ABI_NATIVE)/$(OBJDIR)/xap_$(ABI_FE)Slurp.$(OBJ_SUFFIX)
endif

OBJS=		$(PLATFORM_OBJS)						\
		xp/$(OBJDIR)/xad_Document.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/xap_App.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/xap_Args.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/xap_Clipboard.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xap_Dialog.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/xap_DialogFactory.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xap_Dlg_About.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xap_Dlg_FileOpenSaveAs.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/xap_Dlg_FontChooser.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xap_Dlg_MessageBox.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xap_Dlg_Print.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xap_Dlg_WindowMore.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xap_Dlg_Zoom.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/xap_FakeClipboard.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xap_Frame.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/xap_LoadBindings.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xap_Prefs.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/xap_Scrollbar_ViewListener.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/xap_Strings.$(OBJ_SUFFIX)				\
		xp/$(OBJDIR)/xap_Toolbar_ControlFactory.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/xap_ViewListener.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/xav_View.$(OBJ_SUFFIX)

include $(ABI_DEPTH)/config/abi_rules.mk


build:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(TARGETS) $(DIST)/lib
