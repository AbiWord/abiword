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

CPPSRCS=		ie_exp.cpp				\
			ie_exp_AbiWord_1.cpp			\
			ie_exp_Applix.cpp			\
			ie_exp_GZipAbiWord.cpp                  \
			ie_exp_HTML.cpp				\
		        ie_exp_MsWord_97.cpp                    \
			ie_exp_Text.cpp				\
			ie_exp_RTF.cpp				\
			ie_exp_RTF_listenerGetProps.cpp		\
			ie_exp_RTF_listenerWriteDoc.cpp		\
			ie_exp_UTF8.cpp				\
			ie_exp_LaTeX.cpp			\
			ie_exp_PalmDoc.cpp			\
			ie_exp_WML.cpp				\
			ie_exp_DocBook.cpp			\
			ie_exp_Psion.cpp			\
			ie_exp_MIF.cpp				\
			ie_exp_XSL-FO.cpp			\
			ie_imp.cpp				\
			ie_imp_XML.cpp				\
			ie_imp_AbiWord_1.cpp			\
			ie_imp_GZipAbiWord.cpp                  \
			ie_imp_DocBook.cpp                      \
			ie_imp_MsWord_97.cpp			\
			ie_imp_PalmDoc.cpp			\
			ie_imp_RTF.cpp				\
			ie_imp_Text.cpp				\
			ie_imp_UTF8.cpp				\
			ie_imp_WML.cpp				\
			ie_imp_XHTML.cpp			\
			ie_imp_XSL-FO.cpp			\
			ie_imp_Applix.cpp			\
			ie_imp_MIF.cpp				\
			ie_imp_GraphicAsDocument.cpp		\
			ie_impGraphic.cpp			\
			ie_impGraphic_PNG.cpp			\
			ie_impGraphic_SVG.cpp			\
			ie_impGraphic_BMP.cpp			\
			ie_imp_Psion.cpp


LIBRARY_NAME=		AbiImpExp
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

ifeq ($(OS_NAME), WIN32) 
INCLUDES= -I$(ABI_XX_ROOT)/../wv -I$(ABI_XX_ROOT)/../wv/exporter -I$(ABI_XX_ROOT)/../zlib -I$(ABI_XX_ROOT)/../wv/libole2 -I$(ABI_XX_ROOT)/../libpng -I$(ABI_XX_ROOT)/../libiconv/include -I$(ABI_XX_ROOT)/../psiconv
else 
INCLUDES= -I$(ABI_ROOT)/../wv -I$(ABI_ROOT)/../wv/exporter -I$(ABI_ROOT)/../wv/libole2 -I$(ABI_ROOT)/../libpng -I$(ABI_ROOT)/../psiconv
endif 

ifdef GLIB_CONFIG 
else 
 ifeq ($(OS_NAME), WIN32) 
  INCLUDES+= -I$(ABI_XX_ROOT)/../wv/glib-wv 
 else 
  INCLUDES+= -I$(ABI_ROOT)/../wv/glib-wv 
 endif 
endif 

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
