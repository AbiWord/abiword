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

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

## TODO: decide if all of these need to be exported.
## TODO: move the ones which dont to the HEADERS definition

EXPORT_HEADERS=		ap_EditMethods.h		\
			ap_LoadBindings.h		\
			ap_Menu_ActionSet.h		\
			ap_Menu_Functions.h		\
			ap_Menu_LabelSet.h		\
			ap_Menu_Layouts.h		\
			ap_Menu_Id.h

HEADERS=		$(EXPORT_HEADERS)		\
			ap_LoadBindings_Default.h	\
			ap_Menu_LabelSet_EnUS.h		\
			ap_Menu_LabelSet_Languages.h	\
			ap_Menu_Layouts.h		\
			ap_Menu_Layouts_All.h		\
			ap_Menu_Layouts_DebugMenu.h	\
			ap_Menu_Layouts_MainMenu.h

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		ap_EditMethods.cpp		\
			ap_LoadBindings.cpp		\
			ap_LoadBindings_Default.cpp	\
			ap_Menu_ActionSet.cpp		\
			ap_Menu_Functions.cpp		\
			ap_Menu_LabelSet.cpp		\
			ap_Menu_Layouts.cpp

INCLUDES=		-I$(DIST)/include -I. -DUT_DEBUG

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

export:: $(EXPORT_HEADERS)
	@echo Exporting headers from `pwd`...
	@$(subst xxxx,$(DIST)/include,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(EXPORT_HEADERS) $(DIST)/include

libs:: $(TARGETS)
