#! gmake

## AbiWord
## Copyright (C) 1998-2000 AbiSource, Inc.
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

ifeq ($(ABI_OPT_GNOME),1)
DIRS=		$(ABI_GNOME_DIR)
endif


CPPSRCS=		ie_impexp_RegisterUnix.cpp \
			ie_impGraphic_GdkPixbuf.cpp


TARGETS=                $(OBJS)


INCLUDES+= -I$(ABI_ROOT)/../wv -I$(ABI_ROOT)/../wv/exporter -I$(ABI_ROOT)/../libpng -I$(ABI_ROOT)/../psiconv

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
