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

ABI_ROOT:=$(shell pwd)/../../..
include $(ABI_ROOT)/src/config/abi_defs_wp.mk

DIRS=		xp $(ABI_NATIVE)
LIBRARY_NAME=           AbiImpExp
LIBRARY_VERSION=        $(ABI_VERSION)
TARGETS=                $(LIBRARY)

PLATFORM_OBJS= $(OBJDIR)/ie_impexp_Register$(ABI_FE).$(OBJ_SUFFIX)

ifeq ($(ABI_NATIVE), unix)
	ifeq ($(ABI_OPT_GNOME),1)
		PLATFORM_OBJS+= $(OBJDIR)/ie_impGraphic_GdkPixbuf.$(OBJ_SUFFIX)  \
			$(OBJDIR)/ie_impexp_Register$(ABI_GNOME_PREFIX).$(OBJ_SUFFIX)
	endif
endif

OBJS=	$(PLATFORM_OBJS)	\
	$(OBJDIR)/ie_impexp_Register.$(OBJ_SUFFIX)                  \
        $(OBJDIR)/ie_impexp_MSWrite.$(OBJ_SUFFIX)                   \
        $(OBJDIR)/ie_exp.$(OBJ_SUFFIX)                              \
        $(OBJDIR)/ie_exp_AbiWord_1.$(OBJ_SUFFIX)                    \
        $(OBJDIR)/ie_exp_AWT.$(OBJ_SUFFIX)                          \
        $(OBJDIR)/ie_exp_Applix.$(OBJ_SUFFIX)                       \
        $(OBJDIR)/ie_exp_GZipAbiWord.$(OBJ_SUFFIX)                  \
        $(OBJDIR)/ie_exp_HTML.$(OBJ_SUFFIX)                         \
        $(OBJDIR)/ie_exp_MsWord_97.$(OBJ_SUFFIX)                    \
        $(OBJDIR)/ie_exp_Text.$(OBJ_SUFFIX)                         \
        $(OBJDIR)/ie_exp_HRText.$(OBJ_SUFFIX)                       \
        $(OBJDIR)/ie_exp_RTF.$(OBJ_SUFFIX)                          \
        $(OBJDIR)/ie_exp_RTF_AttrProp.$(OBJ_SUFFIX)                 \
        $(OBJDIR)/ie_exp_RTF_listenerGetProps.$(OBJ_SUFFIX)         \
        $(OBJDIR)/ie_exp_RTF_listenerWriteDoc.$(OBJ_SUFFIX)         \
        $(OBJDIR)/ie_exp_LaTeX.$(OBJ_SUFFIX)                        \
        $(OBJDIR)/ie_exp_PalmDoc.$(OBJ_SUFFIX)                      \
        $(OBJDIR)/ie_exp_WML.$(OBJ_SUFFIX)                          \
        $(OBJDIR)/ie_exp_DocBook.$(OBJ_SUFFIX)                      \
        $(OBJDIR)/ie_exp_Psion.$(OBJ_SUFFIX)                        \
        $(OBJDIR)/ie_exp_MIF.$(OBJ_SUFFIX)                          \
        $(OBJDIR)/ie_exp_XSL-FO.$(OBJ_SUFFIX)                       \
        $(OBJDIR)/ie_exp_ISCII.$(OBJ_SUFFIX)                        \
        $(OBJDIR)/ie_exp_PW.$(OBJ_SUFFIX)                           \
        $(OBJDIR)/ie_exp_KWord_1.$(OBJ_SUFFIX)                      \
        $(OBJDIR)/ie_imp.$(OBJ_SUFFIX)                              \
        $(OBJDIR)/ie_imp_XML.$(OBJ_SUFFIX)                          \
        $(OBJDIR)/ie_imp_AbiWord_1.$(OBJ_SUFFIX)                    \
        $(OBJDIR)/ie_imp_GZipAbiWord.$(OBJ_SUFFIX)                  \
        $(OBJDIR)/ie_imp_DocBook.$(OBJ_SUFFIX)                      \
        $(OBJDIR)/ie_imp_MsWord_97.$(OBJ_SUFFIX)                    \
        $(OBJDIR)/ie_imp_PalmDoc.$(OBJ_SUFFIX)                      \
        $(OBJDIR)/ie_imp_KWord_1.$(OBJ_SUFFIX)                      \
        $(OBJDIR)/ie_imp_RTF.$(OBJ_SUFFIX)                          \
        $(OBJDIR)/ie_imp_Text.$(OBJ_SUFFIX)                         \
        $(OBJDIR)/ie_imp_WML.$(OBJ_SUFFIX)                          \
        $(OBJDIR)/ie_imp_XHTML.$(OBJ_SUFFIX)                        \
        $(OBJDIR)/ie_imp_XSL-FO.$(OBJ_SUFFIX)                       \
        $(OBJDIR)/ie_imp_Applix.$(OBJ_SUFFIX)                       \
        $(OBJDIR)/ie_imp_MIF.$(OBJ_SUFFIX)                          \
        $(OBJDIR)/ie_imp_GraphicAsDocument.$(OBJ_SUFFIX)            \
        $(OBJDIR)/ie_impGraphic.$(OBJ_SUFFIX)                       \
        $(OBJDIR)/ie_impGraphic_PNG.$(OBJ_SUFFIX)                   \
        $(OBJDIR)/ie_impGraphic_SVG.$(OBJ_SUFFIX)                   \
        $(OBJDIR)/ie_impGraphic_BMP.$(OBJ_SUFFIX)                   \
        $(OBJDIR)/ie_imp_Psion.$(OBJ_SUFFIX)                        \
        $(OBJDIR)/ie_imp_PW.$(OBJ_SUFFIX)                           \
        $(OBJDIR)/ie_imp_WordPerfect.$(OBJ_SUFFIX)                  \
        $(OBJDIR)/ie_imp_MSWrite.$(OBJ_SUFFIX)                      \
        $(OBJDIR)/ie_imp_ISCII.$(OBJ_SUFFIX)                        \
        $(OBJDIR)/ie_imp_T602.$(OBJ_SUFFIX)


include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)

