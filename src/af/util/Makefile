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

ABI_DEPTH=		..
include $(ABI_DEPTH)/config/abi_defs.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiUtil
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

# note: we assume that this will be used as a static library
# note: only.  if we want to use it as a shared library, then
# note: we probably need to append $(SHARED_LIBRARY) to TARGETS
# note: and add a line to the intall section which copies it to
# note: the $(DIST)/bin directory on Windows.

ifeq ($(ABI_NATIVE), unix)
## TODO fix capitalization and change to $(ABI_FE)
PLATFORM_OBJS=	$(ABI_NATIVE)/$(OBJDIR)/ut_unixAssert.$(OBJ_SUFFIX)	\
		$(ABI_NATIVE)/$(OBJDIR)/ut_unixString.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ut_unixTimer.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ut_dialogHelper.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_NATIVE), win)
PLATFORM_OBJS=	$(ABI_NATIVE)/$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)	\
		$(ABI_NATIVE)/$(OBJDIR)/ut_$(ABI_FE)OS.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ut_$(ABI_FE)String.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ut_$(ABI_FE)Timer.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ut_Xpm2Bmp.$(OBJ_SUFFIX)
endif

ifeq ($(ABI_NATIVE), beos)
## TODO fix capitalization and change to $(ABI_FE)
PLATFORM_OBJS=	$(ABI_NATIVE)/$(OBJDIR)/ut_$(ABI_FE)Assert.$(OBJ_SUFFIX)	\
		$(ABI_NATIVE)/$(OBJDIR)/ut_$(ABI_FE)String.$(OBJ_SUFFIX)	\
		$(ABI_NATIVE)/$(OBJDIR)/ut_$(ABI_FE)Timer.$(OBJ_SUFFIX)		\
		$(ABI_NATIVE)/$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)
endif

### add this to the OBJS when we need it for calc.
###		xp/$(OBJDIR)/ut_regex.$(OBJ_SUFFIX)			\

OBJS=		$(PLATFORM_OBJS)						\
		xp/$(OBJDIR)/ut_alphahash.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ut_base64.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ut_base64_TestRoutines.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ut_bytebuf.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ut_growbuf.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ut_hash.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_misc.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_png.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_pool.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_stack.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_string.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ut_test.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_timer.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_units.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_vector.$(OBJ_SUFFIX)

include $(ABI_DEPTH)/config/abi_rules.mk


build:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(TARGETS) $(DIST)/lib
