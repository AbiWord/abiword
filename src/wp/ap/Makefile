#! gmake
 
##
## The contents of this file are subject to the AbiSource Public
## License Version 1.0 (the "License"); you may not use this file
## except in compliance with the License. You may obtain a copy
## of the License at http://www.abisource.com/LICENSE/ 
## 
## Software distributed under the License is distributed on an
## "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
## implied. See the License for the specific language governing
## rights and limitations under the License. 
## 
## The Original Code is AbiSource Utilities.
## 
## The Initial Developer of the Original Code is AbiSource, Inc.
## Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
## All Rights Reserved. 
## 
## Contributor(s):
##  

ABI_DEPTH=		../..
include $(ABI_DEPTH)/config/abi_defs.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiAp
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

# note: we assume that this will be used as a static library
# note: only.  if we want to use it as a shared library, then
# note: we probably need to append $(SHARED_LIBRARY) to TARGETS
# note: and add a line to the intall section which copies it to
# note: the $(DIST)/bin directory on Windows.

ifeq ($(ABI_NATIVE), win)
PLATFORM_OBJS=
endif
ifeq ($(ABI_NATIVE), unix)
PLATFORM_OBJS=	unix/$(OBJDIR)/ap_UnixAp.$(OBJ_SUFFIX)			\
		unix/$(OBJDIR)/ap_UnixFrame.$(OBJ_SUFFIX)
endif

OBJS=		$(PLATFORM_OBJS)					\
		xp/$(OBJDIR)/ap_Ap.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_EditMethods.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_Frame.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ap_LoadBindings.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_LoadBindings_Default.$(OBJ_SUFFIX)	\
		xp/$(OBJDIR)/ap_Menu_ActionSet.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_Menu_Functions.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_Menu_LabelSet.$(OBJ_SUFFIX)		\
		xp/$(OBJDIR)/ap_Menu_Layouts.$(OBJ_SUFFIX)

include $(ABI_DEPTH)/config/abi_rules.mk


libs:: $(TARGETS)

install:: libs
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(TARGETS) $(DIST)/lib
