#! gmake

## AbiWord
## Copyright (C) 1998 AbiSource, Inc.
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Library General Public
## License as published by the Free Software Foundation; either
## version 2 of the License, or (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Library General Public License for more details.
##
## You should have received a copy of the GNU Library General Public
## License along with this library; if not, write to the
## Free Software Foundation, Inc., 59 Temple Place - Suite 330,
## Boston, MA 02111-1307, USA.

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

EXPORT_HEADERS=		fv_View.h	\
			fl_DocLayout.h	\
			fl_Types.h		

HEADERS=		$(EXPORT_HEADERS)	\
			fb_LineBreaker.h	\
			fl_BlockLayout.h	\
			fl_ColumnLayout.h	\
			fl_ColumnSetLayout.h	\
			fl_SectionLayout.h	\
			fl_DocListener.h		\
			fl_Layout.h		\
			fp_Column.h		\
			fp_BlockSlice.h		\
			fp_Line.h		\
			fp_Page.h		\
			fp_Run.h		\
			fp_SectionSlice.h

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		fv_View.cpp		\
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

INCLUDES=		-I$(DIST)/include -I. -DUT_DEBUG -DPT_TEST

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
