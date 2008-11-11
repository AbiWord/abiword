#! gmake

## AbiSource Program Utilities
## Copyright (C) 1998 AbiSource, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
#
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

include $(ABI_ROOT)/src/config/abi_defs.mk

CPPSRCS=	ut_base64.cpp		\
		ut_base64_TestRoutines.cpp	\
		ut_allocator.cpp	\
		ut_rand.cpp		\
		ut_worker.cpp		\
		ut_idle.cpp		\
		ut_bytebuf.cpp		\
		ut_growbuf.cpp		\
		ut_hash.cpp		\
		ut_iconv.cpp		\
		ut_math.cpp		\
		ut_mbtowc.cpp		\
		ut_misc.cpp		\
		ut_png.cpp		\
		ut_locale.cpp		\
		ut_svg.cpp		\
		ut_xml.cpp		\
		ut_Script.cpp		\
		ut_stack.cpp		\
		ut_string.cpp		\
		ut_test.cpp		\
		ut_timer.cpp		\
		ut_types.cpp		\
		ut_units.cpp		\
		ut_vector.cpp		\
		ut_PropVector.cpp	\
		ut_wctomb.cpp		\
		ut_bijection.cpp	\
		ut_string_class.cpp	\
		ut_stringbuf.cpp  	\
		ut_AdobeEncoding.cpp 	\
		ut_OverstrikingChars.cpp \
		ut_Language.cpp		\
		ut_Encoding.cpp     	\
		ut_decompress.cpp	\
                ut_crc32.cpp		\
		ut_uuid.cpp		\
		ut_go_file.cpp \
		ut_html.cpp \
		ut_unicode.cpp

ifeq ($(ABI_OPT_PEER_EXPAT),1)
 CPPSRCS += ut_xml_expat.cpp
else
	ifneq ($(ABI_OPT_MSXML),1)
		CPPSRCS += ut_xml_libxml2.cpp
	endif
endif

ifeq ($(ABI_OPT_PERL),1)
CPPSRCS += ut_PerlBindings.cpp
endif

### regex is only used by calc.
###CSRCS=		ut_regex.c

TARGETS=		$(OBJS)

ifeq ($(ABI_NATIVE),win)
INCLUDES = -I$(ABI_XX_ROOT)/../libpng -I$(ABI_ZLIB_INC) -I$(ABI_XX_ROOT)/../libiconv/include $(ABI_GLIB_INC)
endif

ifeq ($(OS_NAME),WIN32)
INCLUDES += -GX
endif

ifeq ($(ABI_NATIVE), qnx)
INCLUDES = -I$(ABI_ROOT)/../libpng -I$(ABI_ROOT)/../zlib
endif

#This can't really hurt anything
#INCLUDES += -I$(ABI_ROOT)/../libpng -I$(ABI_ROOT)/../zlib

# If you add another operating system that does require
# libpng, add lines like this to the "ifeq" group above.

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)



