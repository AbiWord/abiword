#! gmake

## AbiWord
## Copyright (C) 1998 AbiSource, Inc.
## Copyright (C) 2008 AbiSource Corporation B.V.
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

ABI_ROOT:=$(shell pwd)/../../../../../../abi
include $(ABI_ROOT)/src/config/abi_defs.mk

TARGETS=			$(OBJS)
CPPSRCS=			ap_Win32Dialog_GenericInput.cpp \
				ap_Win32Dialog_GenericProgress.cpp \
				ServiceWin32AccountHandler.cpp			

RCSRCS=			ap_Win32Res_ServiceDialogs.rc

INCLUDES+=			-I$(shell pwd) \
				-I$(shell pwd)/../../.. \
				-I$(shell pwd)/../../../core/packet \
				-I$(shell pwd)/../../../core/session \
				-I$(shell pwd)/../../../core/dialogs \
				-I$(ABI_ROOT)/../libs/boost \
				-I$(ABI_ROOT)/../libs/asio/include

#CFLAGS += -DABI_DLL

ifeq ($(ABI_FE), Win32)
# common to all Win32 compilers

# compiler (MinGW, MSVC, etc) specific
ifeq ($(OS_NAME),MINGW32)
else
	OS_CFLAGS+=-GX
endif
endif

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
