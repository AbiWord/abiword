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
## The Original Code is AbiWord.
## 
## The Initial Developer of the Original Code is AbiSource, Inc.
## Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
## All Rights Reserved. 
## 
## Contributor(s):
##  

ABI_DEPTH = ../..

include $(ABI_DEPTH)/config/abi_defs.mk

## TODO: decide if all of these need to be exported.
## TODO: move the ones which dont to the HEADERS definition

EXPORT_HEADERS=		ev_EditBinding.h	\
			ev_EditBits.h		\
			ev_EditEventMapper.h	\
			ev_EditMethod.h		\
			ev_Keyboard.h		\
			ev_Menu.h		\
			ev_Menu_Actions.h	\
			ev_Menu_Labels.h	\
			ev_Menu_Layouts.h	\
			ev_Mouse.h		\
			ev_NamedVirtualKey.h	

HEADERS=		$(EXPORT_HEADERS)

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		ev_EditBinding.cpp		\
			ev_EditEventMapper.cpp		\
			ev_EditMethod.cpp		\
			ev_Keyboard.cpp			\
			ev_Menu.cpp			\
			ev_Menu_Actions.cpp		\
			ev_Menu_Labels.cpp		\
			ev_Menu_Layouts.cpp		\
			ev_Mouse.cpp			\
			ev_NamedVirtualKey.cpp		


INCLUDES=		-I$(DIST)/include -I. -DUT_DEBUG

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

export:: $(EXPORT_HEADERS)
	@echo Exporting headers from `pwd`...
	@$(subst xxxx,$(DIST)/include,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(EXPORT_HEADERS) $(DIST)/include

libs:: $(TARGETS)
