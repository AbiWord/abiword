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

##################################################################
##################################################################
## abi_defs.mk --  Makefile definitions for building AbiSource software.
## This is a makefile include.  It should be included after ABI_DEPTH
## is set and before any other declarations.
##
## The general structure of an AbiSource Makefile should be:
##
##        #! gmake
##        ABI_DEPTH=<your depth in source tree from abi/src>
##        include $(ABI_DEPTH)/config/abi_defs.mk
##        <local declarations>
##        include $(ABI_DEPTH)/config/abi_rules.mk
##        <local rules>
##
##################################################################
##################################################################


##################################################################
##################################################################
## Interlude into NSPR makefile system.
## 
## Map ABI_DEPTH (set in makefile that included us) into NSPR's
## MOD_DEPTH, include their config.mk, and fix up any paths as
## necessary.

MOD_DEPTH=$(ABI_DEPTH)/other/nsprpub
include $(MOD_DEPTH)/config/config.mk


##################################################################
##################################################################
## Define ABI_ symbols to help with cross-platform (xp) and
## platform-dependent directory naming.

ifeq ($(OS_ARCH),WINNT)
ABI_NATIVE=	win
else
ABI_NATIVE=	unix
endif

##################################################################
##################################################################
## Define AbiSoftware version

ABI_VERSION=	0_0


##################################################################
##################################################################
## Help for the loader.  In the makefile which builds the program,
## the following three variables:
##
##    ABI_APPLIBS should be for ABI_ versioned things in $(DIST)/lib
##    ABI_OTHLIBS should be for MOD_ versioned things in $(DIST)/lib (from abi/src/other)
##    ABI_LIBS should be for the X11 libraries and the like

ifeq ($(OS_ARCH),WINNT)
ifeq ($(BUILDWXWIN),)
EXTRA_LIBS=	$(addprefix $(DIST)/lib/lib,$(addsuffix $(ABI_VERSION)_s.lib,$(ABI_APPLIBS)))	\
		$(addprefix $(DIST)/lib/lib,$(addsuffix $(MOD_VERSION)_s.lib,$(ABI_OTHLIBS)))	\
		$(addsuffix .lib,$(ABI_LIBS))
else
ifeq ($(WXINCDIR),)
bogusincdir:
	@echo To compile the WXWindows version of Abiword, you must set the following
	@echo variables in your environment:
	@echo    WXLIBDIR - directory where the wx.lib library is
	@echo    WXINCDIR - directory for wxwin include files
	@echo
	@echo an example for bash:
	@echo export WXLIBDIR=c:/wxwin/lib
	@echo export WXINCDIR=c:/wxwin/include
	@echo
	@echo for csh/tsch:
	@echo setenv WXLIBDIR c:/wxwin/lib
	@echo setenv WXINCDIR c:/wxwin/include
	@echo set BUILDWXWIN=something to build the wxWindows version
	@echo
	@echo unset the BUILDWXWIN variable in your environment to not build with wxWin
endif
ifeq ($(WXLIBDIR),)
boguslibdir:
	@echo To compile the WXWindows version of Abiword, you must set the following
	@echo variables in your environment:
	@echo    WXLIBDIR - directory where the wx.lib library is
	@echo    WXINCDIR - directory for wxwin include files
	@echo
	@echo an example for bash:
	@echo export WXLIBDIR=c:/wxwin/lib
	@echo export WXINCDIR=c:/wxwin/include
	@echo
	@echo for csh/tsch:
	@echo setenv WXLIBDIR c:/wxwin/lib
	@echo setenv WXINCDIR c:/wxwin/include
	@echo
	@echo unset the BUILDWXWIN variable in your environment to not build with wxWin
endif
EXTRA_LIBS=	$(addprefix $(DIST)/lib/lib,$(addsuffix $(ABI_VERSION)_s.lib,$(ABI_APPLIBS)))	\
		$(addprefix $(DIST)/lib/lib,$(addsuffix $(MOD_VERSION)_s.lib,$(ABI_OTHLIBS)))	\
		$(addsuffix .lib,$(ABI_LIBS))	\
		$(WXLIBDIR)/wx.lib

WXINCLUDE=	-D__WXMSW__ -D__WIN95__ -I$(WXINCDIR)
endif
else
ifeq ($(BUILDWXWIN),)
EXTRA_LIBS=	-L$(DIST)/lib 							\
		$(addprefix -l,$(addsuffix $(ABI_VERSION),$(ABI_APPLIBS)))	\
		$(addprefix -l,$(addsuffix $(MOD_VERSION),$(ABI_OTHLIBS)))	\
		$(addprefix -l,$(ABI_LIBS))	\
		`gtk-config --libs`
else
ifeq ($(WXINCDIR),)
bogusincdir:
	@echo To compile the WXWindows version of Abiword, you must set the following
	@echo variables in your environment:
	@echo    WXLIBDIR - directory where the wx library is
	@echo    WXINCDIR - directory for wxwin include files
	@echo
	@echo an example for bash:
	@echo export WXLIBDIR=/usr/src/wxGTK/lib/Linux
	@echo export WXINCDIR=/usr/src/wxGTK/include
	@echo
	@echo for csh/tsch:
	@echo setenv WXLIBDIR /usr/src/wxGTK/lib/Linux
	@echo setenv WXINCDIR /usr/src/wxGTK/include
	@echo
	@echo unset the BUILDWXWIN variable in your environment to not build with wxWin
endif
ifeq ($(WXLIBDIR),)
boguslibdir:
	@echo To compile the WXWindows version of Abiword, you must set the following
	@echo variables in your environment:
	@echo    WXLIBDIR - directory where the wx library is
	@echo    WXINCDIR - directory for wxwin include files
	@echo
	@echo an example for bash:
	@echo export WXLIBDIR=/usr/src/wxGTK/lib/Linux
	@echo export WXINCDIR=/usr/src/wxGTK/include
	@echo
	@echo for csh/tsch:
	@echo setenv WXLIBDIR /usr/src/wxGTK/lib/Linux
	@echo setenv WXINCDIR /usr/src/wxGTK/include
	@echo
	@echo unset the BUILDWXWIN variable in your environment to not build with wxWin
endif
EXTRA_LIBS=	-L$(DIST)/lib 						\
		$(addprefix -l,$(addsuffix $(ABI_VERSION),$(ABI_APPLIBS))) \
		$(addprefix -l,$(addsuffix $(MOD_VERSION),$(ABI_OTHLIBS))) \
		$(addprefix -l,$(ABI_LIBS))	\
		`gtk-config --libs`	\
		-L$(WXLIBDIR) -lwx_gtk

WXINCLUDE=	-D__WXGTK__ -I$(WXINCDIR)
endif
endif

##################################################################
##################################################################

