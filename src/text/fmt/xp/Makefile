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

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

CPPSRCS=		fv_View.cpp			\
			fv_View_TestRoutines.cpp	\
			fb_LineBreaker.cpp		\
			fl_BlockLayout.cpp		\
			fl_DocLayout.cpp		\
			fl_DocListener.cpp		\
			fl_Layout.cpp			\
			fl_SectionLayout.cpp		\
			fp_Column.cpp			\
			fp_Line.cpp			\
			fp_Page.cpp			\
			fp_Run.cpp

LIBRARY_NAME=		AbiFmt
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

include $(ABI_DEPTH)/config/abi_rules.mk

build:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(TARGETS) $(DIST)/lib
