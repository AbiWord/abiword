#! gmake

## AbiSource Program Utilities
## Copyright (C) 1998 AbiSource, Inc.
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Library General Public
## License as published by the Free Software Foundation; either
## version 2 of the License, or (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Library General Public License for more details.
##
## You should have received a copy of the GNU Library General Public
## License along with this library; if not, write to the
## Free Software Foundation, Inc., 59 Temple Place - Suite 330,
## Boston, MA 02111-1307, USA.

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

DISTBASE = $(ABI_DEPTH)/../dist

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
EXTRA_LIBS=	$(addprefix $(DIST)/lib/lib,$(addsuffix $(ABI_VERSION)_s.lib,$(ABI_APPLIBS)))	\
		$(addprefix $(DIST)/lib/lib,$(addsuffix $(MOD_VERSION)_s.lib,$(ABI_OTHLIBS)))	\
		$(addsuffix .lib,$(ABI_LIBS))
else
EXTRA_LIBS=	-L$(DIST)/lib 							\
		$(addprefix -l,$(addsuffix $(ABI_VERSION),$(ABI_APPLIBS)))	\
		$(addprefix -l,$(addsuffix $(MOD_VERSION),$(ABI_OTHLIBS)))	\
		$(addprefix -l,$(ABI_LIBS))	\
		`gtk-config --libs`
endif

##################################################################

