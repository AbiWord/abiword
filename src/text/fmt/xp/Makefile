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

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs_wp.mk

CPPSRCS=		fv_View.cpp			\
			fv_View_TestRoutines.cpp	\
			fb_LineBreaker.cpp		\
			fb_Alignment.cpp		\
			fl_BlockLayout.cpp		\
			fl_DocLayout.cpp		\
			fl_DocListener.cpp		\
			fl_Layout.cpp			\
			fl_SectionLayout.cpp		\
			fl_AutoNum.cpp			\
			fl_TestRoutines.cpp		\
			fp_Column.cpp			\
			fp_Line.cpp			\
			fp_Page.cpp			\
			fp_PageSize.cpp			\
			fp_Run.cpp			\
			fp_FmtMarkRun.cpp		\
			fp_TextRun.cpp			\
			fp_TestRoutines.cpp		\
			fg_Graphic.cpp			\
			fg_GraphicRaster.cpp		\
			fg_GraphicVector.cpp		\
			fp_FieldListLabelRun.cpp

LIBRARY_NAME=		AbiFmt
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

include $(ABI_ROOT)/src/config/abi_rules.mk

# Win32 must build libiconv as a peer, Unix should either
# have iconv.h available as part of its C library (GNU libc
# systems) or have it installed as part of the libiconv-1.0
# package.

# Test for iconv in system locations
HAVE_ICONV_SYSTEM := $(shell if [ -r /usr/include/iconv.h -o -r /usr/local/include/iconv.h ] ; then echo 1 ; fi)

ifeq ($(OS_NAME), WIN32)
INCLUDES=		-I$(ABI_XX_ROOT)/../libiconv/include
else
ifneq ($(HAVE_ICONV_SYSTEM),1)
INCLUDES= 		-I$(ABI_ROOT)/../libiconv/include
endif
endif

build:: $(TARGETS)
