#! gmake

## AbiSource Program Utilities
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

CPPSRCS=		abi_ver.cpp

LIBRARY_NAME=		AbiWordVer
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

ifdef ABI_BUILD_ID
BUILD_ID=		$(ABI_BUILD_ID)
else
BUILD_ID=		
endif

ifdef ABI_BUILD_VERSION
BUILD_VERSION=		$(ABI_BUILD_VERSION)
else
BUILD_VERSION=
endif

ifdef ABI_OPTIONS
ifeq ($(OS_NAME),WIN32)
BUILD_OPTIONS=		\"$(ABI_OPTIONS)\"
else
BUILD_OPTIONS=		"$(ABI_OPTIONS)"
endif
else
BUILD_OPTIONS=
endif

ABI_VERFLAGS=		-DABI_BUILD_ID=\"$(BUILD_ID)\"			\
			-DABI_BUILD_VERSION=\"$(BUILD_VERSION)\"	\
			-DABI_BUILD_OPTIONS=\"$(BUILD_OPTIONS)\"	\
			-DABI_BUILD_TARGET=\"$(OBJDIR)\"

INCLUDES=		$(ABI_VERFLAGS)

include $(ABI_DEPTH)/config/abi_rules.mk

build:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(TARGETS) $(DIST)/lib
