#! gmake

## AbiSource Program Utilities
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

ABI_ROOT:=$(shell pwd)/../../..
include $(ABI_ROOT)/src/config/abi_defs.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiUtil
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

ifeq ($(ABI_NATIVE), unix)
## TODO fix capitalization and change to $(ABI_FE)
PLATFORM_OBJS=	$(OBJDIR)/ut_unixAssert.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_unixString.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_unixTimer.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_dialogHelper.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_NATIVE), win)
PLATFORM_OBJS=	$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_$(ABI_FE)OS.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_$(ABI_FE)String.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_$(ABI_FE)Timer.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_iconv.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_Xpm2Bmp.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_NATIVE), beos)
## TODO fix capitalization and change to $(ABI_FE)
PLATFORM_OBJS=	$(OBJDIR)/ut_$(ABI_FE)Assert.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_$(ABI_FE)String.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_$(ABI_FE)Timer.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_Rehydrate.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_Xpm2Bitmap.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_NATIVE), qnx)
## TODO fix capitalization and change to $(ABI_FE)
PLATFORM_OBJS=	$(OBJDIR)/ut_qnxAssert.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_qnxString.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_qnxTimer.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_Xpm2Bitmap.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_qnxHelper.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)
endif

### add this to the OBJS when we need it for calc.
###		$(OBJDIR)/ut_regex.$(OBJ_SUFFIX)			\

ifdef __FreeBSD__

OBJS=		$(PLATFORM_OBJS)						\
		$(OBJDIR)/ut_alphahash.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_base64.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_base64_TestRoutines.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_bytebuf.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_growbuf.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_hash.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_misc.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_png.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_svg.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_pool.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_stack.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_string.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_test.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_timer.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_units.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_vector.$(OBJ_SUFFIX)			

else

OBJS=		$(PLATFORM_OBJS)						\
		$(OBJDIR)/ut_alphahash.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_base64.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_base64_TestRoutines.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_bytebuf.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_growbuf.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_hash.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_mbtowc.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_misc.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_png.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_svg.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_pool.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_stack.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_string.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_test.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_timer.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_units.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_vector.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_wctomb.$(OBJ_SUFFIX)

endif

include $(ABI_ROOT)/src/config/abi_rules.mk


build:: $(TARGETS)
