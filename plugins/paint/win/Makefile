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

ABI_ROOT:=$(shell pwd)/../../../../abi
include $(ABI_ROOT)/src/config/abi_defs.mk

TARGETS=			$(OBJS)
CPPSRCS= 			ut_Win32Process.cpp \
				png2bmp.cpp \
				bmp2png.cpp

CSRCS= 			pngdib.c

CFLAGS += -DABI_DLL

ifeq ($(ABI_FE), Win32)
# common to all Win32 compilers
	INCLUDES += 	-I$(ABI_XX_ROOT)/../libpng -I$(ABI_XX_ROOT)/../zlib
# compiler (MinGW, MSVC, etc) specific
ifeq ($(OS_NAME),MINGW32)
	INCLUDES +=	-I$(ABI_ZLIB_INC)
else
	OS_CFLAGS+=-GX
endif
endif


include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
