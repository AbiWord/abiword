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


OS_NAME		:= $(shell uname -s)
OS_RELEASE	:= $(shell uname -r)
OS_ARCH		:= $(shell uname -m)

DISTBASE = $(ABI_DEPTH)/dist

LINK_DLL	= $(LINK) $(OS_DLLFLAGS) $(DLLFLAGS)

CFLAGS		= $(OPTIMIZER) $(OS_CFLAGS) $(DEFINES) $(INCLUDES) $(XCFLAGS)

INSTALL	= install

ifeq ($(OS_NAME), CYGWIN32_NT)
OS_NAME = WINNT
endif

ifeq ($(OS_NAME), WINNT)

CC = cl
CCC = cl
LINK = link
AR = lib -NOLOGO -OUT:"$@"
RANLIB = echo
BSDECHO = echo
RC = rc.exe

GARBAGE = $(OBJDIR)/vc20.pdb $(OBJDIR)/vc40.pdb

OBJ_SUFFIX = obj
LIB_SUFFIX = lib
DLL_SUFFIX = dll

# do we really need -GT?
OS_CFLAGS = -W3 -nologo -GF -Gy -MDd -GT

OPTIMIZER = -Od -Z7
DEFINES = -DDEBUG -D_DEBUG -UNDEBUG -D_CRTDBG_MAP_ALLOC -DWIN32 -DWINNT -D_X86_
# note that we only build debug.  TODO
DBG_OR_NOT = DBG

DLLFLAGS = -DEBUG -DEBUGTYPE:CV -OUT:"$@"
LDFLAGS = -DEBUG -DEBUGTYPE:CV
OS_DLLFLAGS = -nologo -DLL -SUBSYSTEM:WINDOWS -PDB:NONE

ABI_NATIVE=	win
endif
# end of WinNT section
#######################

ifeq ($(OS_NAME), Linux)

OBJ_SUFFIX	= o
LIB_SUFFIX	= a
DLL_SUFFIX	= so
AR		= ar cr $@

OPTIMIZER	= -g
DEFINES		= -DDEBUG -UNDEBUG
# note that we only build debug.  TODO
DBG_OR_NOT = DBG

CC			= gcc
CCC			= g++
RANLIB			= ranlib

OS_INCLUDES		=
G++INCLUDES		= -I/usr/include/g++

PLATFORM_FLAGS		= -ansi -Wall -pipe -DLINUX -Dlinux
PORT_FLAGS		= -D_POSIX_SOURCE -D_BSD_SOURCE -DHAVE_STRERROR

OS_CFLAGS		= $(DSO_CFLAGS) $(PLATFORM_FLAGS) $(PORT_FLAGS)

PLATFORM_FLAGS		+= -mno-486 -Di386
PORT_FLAGS		+= -D_XOPEN_SOURCE

MKSHLIB			= $(LD) $(DSO_LDOPTS) -soname $(@:$(OBJDIR)/%.so=%.so)
ABI_NATIVE=	unix

endif
# end of Linux section
#######################

define MAKE_OBJDIR
if test ! -d $(@D); then rm -rf $(@D); $(INSTALL) -d $(@D); fi
endef

define VERIFY_DIRECTORY
if test ! -d xxxx; then rm -rf xxxx; $(INSTALL) -d xxxx; fi
endef

OBJDIR = $(OS_NAME)_$(OS_RELEASE)_$(OS_ARCH)_$(DBG_OR_NOT)

# Figure out where the binary code lives.
BUILD		= $(OBJDIR)
DIST		= $(DISTBASE)/$(OBJDIR)

##################################################################
##################################################################
## Define AbiSoftware version

# TODO fix this!
ABI_VERSION=	0_0

##################################################################
##################################################################
## Help for the loader.  In the makefile which builds the program,
## the following three variables:
##
##    ABI_APPLIBS should be for ABI_ versioned things in $(DIST)/lib
##    ABI_OTHLIBS should be for MOD_ versioned things in $(DIST)/lib (from abi/src/other)
##    ABI_LIBS should be for the X11 libraries and the like

ifeq ($(OS_NAME),WINNT)
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
		-L$(WXLIBDIR) -lwx_gtk -ldl

WXINCLUDE=	-D__WXGTK__ -I$(WXINCDIR)
endif
endif

##################################################################
##################################################################

