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


# OS_NAME is the output of uname -s minus any forward slashes
# (so we don't imply another level of depth).  This is to solve
# a problem with BSD/OS.  In fact, it might be good to do this
# to all uname results, so that one doesn't see "sun4/m" as an
# architecture.
OS_NAME		:= $(shell uname -s | sed "s/\//-/")
OS_RELEASE	:= $(shell uname -r | sed "s/\//-/")
OS_ARCH		:= $(shell uname -m | sed "s/\//-/")

# Where to stuff all the bins
DISTBASE 	= $(ABI_DEPTH)/../dist

LINK_DLL	= $(LINK) $(OS_DLLFLAGS) $(DLLFLAGS)

CFLAGS		= $(OPTIMIZER) $(OS_CFLAGS) $(DEFINES) $(INCLUDES) $(XCFLAGS) $(ABI_JSDEFS) $(ABI_VERFLAGS)

INSTALL		= install

ifdef ABI_OPT_JS
ABI_JSLIBS=		js
ABI_JSDEFS=		-DABI_OPT_JS
else
ABI_JSLIBS=
ABI_JSDEFS=
endif

#### Include the proper platform defs.  Add another if clause for
#### any new platforms you port to.

# Defer CYGWIN32 to the normal WIN32 build process
ifeq ($(OS_NAME), CYGWIN32_NT)
OS_NAME = WIN32
endif
ifeq ($(OS_NAME), CYGWIN32_95)
OS_NAME = WIN32
endif

ifeq ($(OS_NAME), WIN32)
include $(ABI_DEPTH)/config/platforms/win32.mk
endif

ifeq ($(OS_NAME), Linux)
include $(ABI_DEPTH)/config/platforms/linux.mk
endif

ifeq ($(OS_NAME), FreeBSD)
include $(ABI_DEPTH)/config/platforms/freebsd.mk
endif

# TODO: how do we differentiate between old SunOS and new Solaris
ifeq ($(OS_NAME), SunOS)
include $(ABI_DEPTH)/config/platforms/sunos.mk
endif
#### End of platform defs

# Generic Unix includes for Gtk, as it moves about installation paths.
# glib/gtk 1.1.X and up stick glibconfig.h in this directory.
ifeq ($(ABI_FE), Unix)
CFLAGS 		+=	`gtk-config --cflags`
endif

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

ifeq ($(OS_NAME),WIN32)
EXTRA_LIBS	= 	$(addprefix $(DIST)/lib/lib,$(addsuffix $(ABI_VERSION)_s.lib,$(ABI_APPLIBS)))	\
			$(addprefix $(DIST)/lib/lib,$(addsuffix $(MOD_VERSION)_s.lib,$(ABI_OTHLIBS)))	\
			$(addsuffix .lib,$(ABI_LIBS))
else
EXTRA_LIBS	=	-L$(DIST)/lib 							\
			$(addprefix -l,$(addsuffix $(ABI_VERSION),$(ABI_APPLIBS)))	\
			$(addprefix -l,$(addsuffix $(MOD_VERSION),$(ABI_OTHLIBS)))	\
			$(addprefix -l,$(ABI_LIBS))					\
			`gtk-config --libs`						
endif

##################################################################

