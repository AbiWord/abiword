#! gmake
 
##
## The contents of this file are subject to the AbiSource Public
## License Version 1.0 (the "License"); you may not use this file
## except in compliance with the License. You may obtain a copy
## of the License at http://www.abisource.com/LICENSE/ 
## 
## Software distributed under the License is distributed on an
## "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
## implied. See the License for the specific language governing
## rights and limitations under the License. 
## 
## The Original Code is AbiWord.
## 
## The Initial Developer of the Original Code is AbiSource, Inc.
## Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
## All Rights Reserved. 
## 
## Contributor(s):
##  

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

## TODO -- shouldn't need all of these
EXPORT_HEADERS=		dg_LayoutView.h	\
			fp_Column.h		\
			fl_SectionLayout.h	\
			fl_DocLayout.h		

HEADERS=		$(EXPORT_HEADERS)	\
			fb_LineBreaker.h	\
			fl_BlockLayout.h	\
			fl_ColumnLayout.h	\
			fl_ColumnSetLayout.h	\
			fl_DocListener.h		\
			fl_Layout.h		\
			fp_BlockSlice.h		\
			fp_Line.h		\
			fp_Page.h		\
			fp_Run.h		\
			fp_SectionSlice.h

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		dg_LayoutView.cpp		\
			fb_LineBreaker.cpp		\
			fl_BlockLayout.cpp		\
			fl_ColumnLayout.cpp		\
			fl_ColumnSetLayout.cpp		\
			fl_DocLayout.cpp		\
			fl_DocListener.cpp		\
			fl_Layout.cpp		\
			fl_SectionLayout.cpp		\
			fp_BlockSlice.cpp		\
			fp_Column.cpp			\
			fp_Line.cpp			\
			fp_Page.cpp			\
			fp_Run.cpp			\
			fp_SectionSlice.cpp

INCLUDES=		-I$(DIST)/include -I. -DUT_DEBUG -DFMTLIB

LIBRARY_NAME=		AbiFmt
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

include $(ABI_DEPTH)/config/abi_rules.mk

export:: $(EXPORT_HEADERS)
	@echo Exporting headers from `pwd`...
	@$(subst xxxx,$(DIST)/include,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(EXPORT_HEADERS) $(DIST)/include

libs:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(TARGETS) $(DIST)/lib

install:: libs
