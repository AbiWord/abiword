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

ABI_DEPTH = ../..

include $(ABI_DEPTH)/config/abi_defs.mk

EXPORT_HEADERS=		ut_assert.h		\
			ut_debugmsg.h		\
			ut_growbuf.h		\
			ut_hash.h		\
			ut_math.h		\
			ut_misc.h		\
			ut_pool.h		\
			ut_stack.h		\
			ut_string.h		\
			ut_test.h		\
			ut_timer.h		\
			ut_types.h		\
			ut_units.h		\
			ut_vector.h

HEADERS=		$(EXPORT_HEADERS)

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		ut_growbuf.cpp		\
			ut_hash.cpp		\
			ut_math.cpp		\
			ut_misc.cpp		\
			ut_pool.cpp		\
			ut_stack.cpp		\
			ut_string.cpp		\
			ut_test.cpp		\
			ut_timer.cpp		\
			ut_units.cpp		\
			ut_vector.cpp

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

build:: $(TARGETS)

