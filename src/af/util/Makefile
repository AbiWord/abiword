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
PLATFORM_OBJS=	$(OBJDIR)/ut_unixAssert.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_unixString.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_unixTimer.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_unixIdle.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_unixMisc.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_unixDirent.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_files.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_NATIVE), mac)
PLATFORM_OBJS= $(OBJDIR)/ut_MacString.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_MacFiles.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_MacAssert.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_MacTimer.$(OBJ_SUFFIX)	\
        $(OBJDIR)/ut_unixAssert.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_NATIVE), win)
PLATFORM_OBJS=	$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_$(ABI_FE)OS.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_$(ABI_FE)String.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_$(ABI_FE)Timer.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_$(ABI_FE)Idle.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_$(ABI_FE)Locale.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_sleep.$(OBJ_SUFFIX)            \
		$(OBJDIR)/ut_Xpm2Bmp.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_Win32Uuid.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_Win32Misc.$(OBJ_SUFFIX)		
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

OBJS=		$(PLATFORM_OBJS)						\
		$(OBJDIR)/ut_base64.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_base64_TestRoutines.$(OBJ_SUFFIX)	\
		$(OBJDIR)/ut_allocator.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_rand.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_bytebuf.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_growbuf.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_hash.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_iconv.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_mbtowc.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_misc.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_png.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_locale.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_svg.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_Script.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_stack.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_string.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_string_class.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_stringbuf.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_test.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_worker.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_idle.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_timer.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_types.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_units.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_bijection.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_vector.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_PropVector.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_wctomb.$(OBJ_SUFFIX)       \
		$(OBJDIR)/ut_AdobeEncoding.$(OBJ_SUFFIX) \
		$(OBJDIR)/ut_OverstrikingChars.$(OBJ_SUFFIX) \
		$(OBJDIR)/ut_Language.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_Encoding.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_path.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_math.$(OBJ_SUFFIX)         \
		$(OBJDIR)/ut_xml.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_decompress.$(OBJ_SUFFIX)		\
		$(OBJDIR)/ut_crc32.$(OBJ_SUFFIX) \
		$(OBJDIR)/ut_uuid.$(OBJ_SUFFIX)			\
		$(OBJDIR)/ut_go_file.$(OBJ_SUFFIX) \
		$(OBJDIR)/ut_html.$(OBJ_SUFFIX) \
		$(OBJDIR)/ut_unicode.$(OBJ_SUFFIX) \

ifeq ($(ABI_OPT_PEER_EXPAT),1)
 OBJS += $(OBJDIR)/ut_xml_expat.$(OBJ_SUFFIX)
else
	ifeq ($(ABI_OPT_MSXML),1)
		OBJS += $(OBJDIR)/ut_xml_msxml.$(OBJ_SUFFIX)
	else
		OBJS += $(OBJDIR)/ut_xml_libxml2.$(OBJ_SUFFIX)
	endif
endif

ifeq ($(ABI_OPT_PERL),1)
OBJS += $(OBJDIR)/ut_PerlBindings.$(OBJ_SUFFIX)
endif

include $(ABI_ROOT)/src/config/abi_rules.mk


build:: $(TARGETS)
