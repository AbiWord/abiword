#! gmake

## AbiWord
## Copyright (C) 2004 Marc Maurer
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

CPPSRCS= \
	rl_ContainerLayout.cpp \
	rl_DocLayout.cpp \
	rl_DocListener.cpp \
	rl_DocSectionLayout.cpp \
	rl_Layout.cpp \
	rl_ParagraphLayout.cpp \
	rl_SectionLayout.cpp \
	rl_SpanLayout.cpp \
	rl_TableLayout.cpp \
	rp_Object.cpp \
	rp_ContainerObject.cpp \
	rp_Paragraph.cpp \
	rp_Run.cpp \
	rp_Section.cpp \
	rp_Table.cpp \
	rp_TextRun.cpp \
	rv_View.cpp


LIBRARY_NAME=		AbiRc
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
