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

ABI_OPT_DEBUG=1		// TODO remove this later

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
# architecture.  The substitutions are taken from the Linux
# kernel Makefile.  The result of this is that even if you have
# a Pentium Pro, you will see your bins in a "...i386" directory.
# This doesn't mean it didn't use your optimizations.

OS_NAME		:= $(shell uname -s | sed "s/\//-/" | sed "s/_/-/" | sed "s/-.*//g")
OS_RELEASE	:= $(shell uname -r | sed "s/\//-/" | sed "s/ .*//g")
OS_ARCH		:= $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ | sed "s/\//-/")

# Where to stuff all the bins
DISTBASE 	= $(ABI_DEPTH)/../dist

ABICOPY=cp

##################################################################
##################################################################
## Help for finding all of our include files without needing to
## export them.
##
##    ABI_INCS is constructed from the following ABI_*_INCS.  Each
##    of these is a directory in our source tree that we should
##    reference for header files.

ABI_XAP_INCS=	/config					\
		/af/xap/xp		/af/xap/$(ABI_NATIVE)	\
		/af/ev/xp		/af/ev/$(ABI_NATIVE)	\
		/af/util/xp		/af/util/$(ABI_NATIVE)	\
		/af/gr/xp		/af/gr/$(ABI_NATIVE)

ABI_OTH_INCS=	/other/spell

ABI_PEER_INCS=	/../../expat/xmlparse	\
		/../../expat/xmltok

###ABI_DIST_INCS=	-I$(ABI_DEPTH)/../dist/$(OBJDIR)/include

ABI_ALL_INCS=	$(ABI_XAP_INCS) $(ABI_PEER_INCS) $(ABI_AP_INCS) $(ABI_OTH_INCS)
ABI_INCS=	$(addprefix -I, $(addprefix $(ABI_DEPTH),$(ABI_ALL_INCS)))

##################################################################
##################################################################

## ABI_OPTIONS is a list of all the conditionally included options
##             suitable for echoing during the build process or
##             including in an AboutBox.

ABI_ENABLED_OPTIONS=

## conditionally enable some additional debugging and test code

ifdef ABI_OPT_DEBUG
ABI_DBGDEFS=		-DUT_DEBUG -DPT_TEST -DFMT_TEST -DUT_TEST
ABI_OPTIONS+=Debug:On
else
ABI_DBGDEFS=		-DNDEBUG
ABI_OPTIONS+=Debug:Off
endif

##################################################################
##################################################################

LINK_DLL	= $(LINK) $(OS_DLLFLAGS) $(DLLFLAGS)

CFLAGS		= $(OPTIMIZER) $(OS_CFLAGS) $(DEFINES) $(INCLUDES) $(XCFLAGS)	\
			$(ABI_DBGDEFS) $(ABI_INCS)

##################################################################
##################################################################
#### Include the proper platform defs.  Add another if clause for
#### any new platforms you port to.

# Defer CYGWIN32 to the normal WIN32 build process
ifeq ($(OS_NAME), CYGWIN32)
OS_NAME = WIN32
endif
ifeq ($(OS_NAME), CYGWIN)
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

ifeq ($(OS_NAME), OpenBSD)
include $(ABI_DEPTH)/config/platforms/openbsd.mk
endif

ifeq ($(OS_NAME), NetBSD)
include $(ABI_DEPTH)/config/platforms/netbsd.mk
endif

ifeq ($(OS_NAME), BeOS)
include $(ABI_DEPTH)/config/platforms/beos.mk
endif

# TODO: how do we differentiate between old SunOS and new Solaris
ifeq ($(OS_NAME), SunOS)
include $(ABI_DEPTH)/config/platforms/sunos.mk
endif

# Catch all for undefined platform (CC will always be defined on a working platform)
ifndef CC
fake-target::
	@echo
	@echo "    I can't seem to figure out which platform you are using."
	@echo
	@echo "    If this is a Unix-like platform, it should be easy to"
	@echo "modify an existing platform configuration file to suit this"
	@echo "platform's needs.  Look in abi/src/config/platforms for starter"
	@echo "platform configuration files, and abi/src/config/abi_defs.mk"
	@echo "for the proper detection magic."
	@echo
	exit 1
endif

#### End of platform defs

# Generic Unix includes for Gtk, as it moves about installation paths.
# glib/gtk 1.1.X and up stick glibconfig.h in this directory.
ifeq ($(ABI_FE), Unix)
CFLAGS 		+=	`gtk-config --cflags`
endif

define MAKE_OBJDIR
if test ! -d $(@D); then rm -rf $(@D); mkdir -p $(@D); fi
endef

define VERIFY_DIRECTORY
if test ! -d xxxx; then rm -rf xxxx; mkdir -p  xxxx; fi
endef

OBJDIR = $(OS_NAME)_$(OS_RELEASE)_$(OS_ARCH)_$(OBJ_DIR_SFX)

# Figure out where the binary code lives.
BUILD		= $(OBJDIR)
DIST		= $(DISTBASE)/$(OBJDIR)

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
			-lpng -lz
endif

#We should change this since not all unix's will use gtk 

ifeq ($(ABI_NATIVE),unix)
EXTRA_LIBS	+=	`gtk-config --libs`
endif

##################################################################

