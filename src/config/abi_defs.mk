#! gmake

## AbiSource Program Utilities
## Copyright (C) 1998,1999 AbiSource, Inc.
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

#### To get a debug build:  add the following line back to the
#### Makefile, add the variable to the make command line, or set
#### this variable as an environment variable.  Note all object
#### files and libraries for debug and non-debug builds will be
#### kept separately.  However, the executable will be overwritten.
#### This may change in the near future.
####
#### NOTE: the Makefiles use 'ifdef' rather than 'ifeq' so setting
#### NOTE: this to **any** value will enable it.
####
#### ABI_OPT_DEBUG=1
####

#### To get a GNOME build:  add the following line back to the
#### Makefile, add the variable to the make command line, or set
#### this variable as an environment variable.  A full recompile
#### must be done when switching the value of this variable.
#### (Each platform makefile (./platforms/*.mk) also has a commented-
#### out note about this incase you only want to enable it on a
#### particular platform.)
####
#### NOTE: the Makefiles use 'ifdef' rather than 'ifeq' so setting
#### NOTE: this to **any** value will enable it.
####
#### ABI_OPT_GNOME=1
####

#### To get a cygwin/gcc/gtk (as opposed to a native win32) build: add
#### the following line back to the Makefile, add the variable to the
#### make command line, or set this variable as an environment
#### variable.  A full recompile must be done when switching the value
#### of this variable.  (Each platform makefile (./platforms/*.mk)
#### also has a commented- out note about this incase you only want to
#### enable it on a particular platform.)
####
#### NOTE: the Makefiles use 'ifdef' rather than 'ifeq' so setting
#### NOTE: this to **any** value will enable it.
####
#### ABI_OPT_CYGWIN_UNIX=1
####

##################################################################
##################################################################
## abi_defs.mk --  Makefile definitions for building AbiSource software.
## This is a makefile include.  It should be included after ABI_ROOT
## is set and before any other declarations.
##
## The general structure of an AbiSource Makefile should be:
##
##        #! gmake
##        ABI_ROOT=<the top-level abi directory>
##        include $(ABI_ROOT)/src/config/abi_defs.mk
##        <local declarations>
##        include $(ABI_ROOT)/src/config/abi_rules.mk
##        <local rules>
##
##################################################################
##################################################################


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

# this makes HP-UX look like "HP" (sed turns "HP-UX" into "HP" with the -.* pattern)
OS_NAME		:= $(shell uname -s | sed "s/\//-/" | sed "s/_/-/" | sed "s/-.*//g")
OS_RELEASE	:= $(shell uname -r | sed "s/\//-/" | sed "s/[() ].*//g")
####OS_ARCH is now set in platform/*.mk

##################################################################
##################################################################
#### Cygnus keeps changing the value that uname returns between
#### different versions of the package and between different
#### versions of Windows.  Here we fold them all in into one symbol.

ifeq ($(OS_NAME), CYGWIN32)
OS_NAME = WIN32
endif
ifeq ($(OS_NAME), CYGWIN)
OS_NAME = WIN32
endif

##################################################################
##################################################################
#### if ABI_OPT_CYGWIN_UNIX is defined then the OS_NAME becomes
#### CYGWIN, and we build with gcc etc.

ifdef ABI_OPT_CYGWIN_UNIX
ifeq ($(OS_NAME), WIN32)
OS_NAME = CYGWIN
endif
endif

##################################################################
##################################################################

ABICOPY=cp

ifdef ABISOURCE_LICENSED_TRADEMARKS
ABI_TMDEFS=	-DABISOURCE_LICENSED_TRADEMARKS
ABI_OPTIONS+=LicensedTrademarks:On
else
ABI_TMDEFS=
ABI_OPTIONS+=LicensedTrademarks:Off
endif

##################################################################
##################################################################
## Help for finding all of our include files without needing to
## export them.
##
##    ABI_INCS is constructed from the following ABI_*_INCS.  Each
##    of these is a directory in our source tree that we should
##    reference for header files.
##
## ABI_XAP_INCS define the cross-platform, cross-application directories
## ABI_OTH_INCS define the header directories in src/other
## ABI_PEEER_INCS define header directories in source trees that are peers to abi
##
## ABI_AP_INCS should define application-specific headers.  these are set
##             in abi_defs_*.mk -- one for each application in AbiSuite.

ABI_XAP_INCS=	/config						\
		/af/xap/xp		/af/xap/$(ABI_NATIVE)	\
		/af/ev/xp		/af/ev/$(ABI_NATIVE)	\
		/af/util/xp		/af/util/$(ABI_NATIVE)	\
		/af/gr/xp		/af/gr/$(ABI_NATIVE)

ifdef ABI_OPT_GNOME
ABI_XAP_INCS+=	/af/xap/$(ABI_NATIVE)/$(ABI_GNOME_DIR)	\
		/af/ev/$(ABI_NATIVE)/$(ABI_GNOME_DIR)
endif

ABI_OTH_INCS=	/other/spell

ABI_PEER_INCS=	/../../expat/xmlparse	\
		/../../expat/xmltok

ABI_ALL_INCS=	$(ABI_XAP_INCS) $(ABI_PEER_INCS) $(ABI_AP_INCS) $(ABI_OTH_INCS) $(ABI_TM_INCS)
ifeq ($(OS_NAME), WIN32)
ABI_XX_ROOT:=$(shell echo $(ABI_ROOT) | sed 's|/cygdrive/\([a-zA-Z]\)/|\1:/|g' | sed 's|//[a-zA-Z]/|/|g')
ABI_INCS=	$(addprefix -I$(ABI_XX_ROOT)/src,$(ABI_ALL_INCS))
else
ABI_INCS=	$(addprefix -I$(ABI_ROOT)/src,$(ABI_ALL_INCS))
endif

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
			$(ABI_TMDEFS) $(ABI_NAMEDEFS) $(ABI_APPLIBDIRDEF)	\
			$(ABI_DBGDEFS) $(ABI_INCS)

##################################################################
##################################################################
#### Include the proper platform defs.  Add another if clause for
#### any new platforms you port to.

ifeq ($(OS_NAME), WIN32)
include $(ABI_ROOT)/src/config/platforms/win32.mk
endif

ifeq ($(OS_NAME), CYGWIN)
include $(ABI_ROOT)/src/config/platforms/cygwin.mk
endif

ifeq ($(OS_NAME), Linux)
include $(ABI_ROOT)/src/config/platforms/linux.mk
endif

ifeq ($(OS_NAME), FreeBSD)
include $(ABI_ROOT)/src/config/platforms/freebsd.mk
endif

ifeq ($(OS_NAME), OpenBSD)
include $(ABI_ROOT)/src/config/platforms/openbsd.mk
endif

ifeq ($(OS_NAME), NetBSD)
include $(ABI_ROOT)/src/config/platforms/netbsd.mk
endif

ifeq ($(OS_NAME), BeOS)
include $(ABI_ROOT)/src/config/platforms/beos.mk
endif

ifeq ($(OS_NAME), procnto)
include $(ABI_ROOT)/src/config/platforms/nto.mk
endif

# TODO: how do we differentiate between old SunOS and new Solaris
ifeq ($(OS_NAME), SunOS)
include $(ABI_ROOT)/src/config/platforms/sunos.mk
endif

ifeq ($(OS_NAME), IRIX)
include $(ABI_ROOT)/src/config/platforms/irix.mk
endif

ifeq ($(OS_NAME), IRIX64)
include $(ABI_ROOT)/src/config/platforms/irix64.mk
endif

# not HP-UX since sed applies s/-.*// to uname
ifeq ($(OS_NAME), HP)
include $(ABI_ROOT)/src/config/platforms/hpux.mk
endif

ifeq ($(OS_NAME), AIX)
include $(ABI_ROOT)/src/config/platforms/aix.mk
endif

ifeq ($(OS_NAME), OSF1)
include $(ABI_ROOT)/src/config/platforms/osf1.mk
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
##################################################################
##################################################################

##################################################################
##################################################################
## Macros which help eliminate our need for a working copy of the
## INSTALL program...

define MAKE_OBJDIR
if test ! -d $(@D); then rm -rf $(@D); mkdir -p $(@D); fi
endef

define VERIFY_DIRECTORY
if test ! -d xxxx; then rm -rf xxxx; mkdir -p  xxxx; fi
endef

define TRANSFORM_TO_DOS_PATH
sed 's|/cygdrive/\([a-zA-Z]\)/|\1:/|g' | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g'
endef

##################################################################
##################################################################
## Directory name pattern and locations of where we put our output.
##
## $OUT/<platform>/{bin,obj}		contains the executables and
##					all other compiler/linker
##					generated stuff.
## $OUT/<platform>/image		contains scratch space used
##					to construct distribution
##					binary images.
## $DIST/				contains the final archives
##					of all distribution binaries.
##
## We just use the abi tree if nothing specified.  Set both
## of these on the command line if you want to keep your
## source tree pristine.

DIST			= $(ABI_ROOT)/dist
OUT			= $(ABI_ROOT)/src

##################################################################
##################################################################
## Symbols to uniquely identify the build.
##
## ABI_BUILD_VERSION	should be set to the build version (1.0.0)
##			for a numbered build.
##
## ABI_BUILD_ID		can be used as a identifying label (such as
##			a date stamp in a nightly build system).
##

ABI_BUILD_VERSION	= unnumbered
ABI_BUILD_ID		=

##################################################################
##################################################################
## ABI_PEER is an empty symbol.  It is used by the various third-
## party libraries to specify a subdirectory name (under OBJDIR)
## of the .o's from the library.  This prevents filename collisions
## between the various libraries that we use.  (For example, both
## zlib and wv have a file called crc32.c -- which are different.)

##################################################################
##################################################################
## set everything else from the above variables....

OUTDIR			= $(OUT)/$(OS_NAME)_$(OS_RELEASE)_$(OS_ARCH)_$(OBJ_DIR_SFX)
OBJDIR			= $(OUTDIR)/obj$(ABI_PEER)
LIBDIR			= $(OUTDIR)/obj
BINDIR			= $(OUTDIR)/bin
CANONDIR		= $(OUTDIR)/AbiSuite

PKGBASENAME		= abisuite-$(ABI_BUILD_VERSION)-$(OS_NAME)_$(OS_ARCH)

USERDIR			= $(ABI_ROOT)/user

##################################################################
##################################################################
## Help for the loader.  In the makefile which builds the actual
## application (abi/src/{wp,show,...}/main/{win,unix,...}/Makefile,
## the following variables:
##
##    ABI_APPLIBS should be for ABI_ versioned things in $(LIBDIR)
##    ABI_LIBS should be for other system libraries
##    ABI_APPLIBDEP should be ABI_APPLIBS without duplicates.
##
## EXTRA_LIBS is the series of -L... -l options needed to link.
## EXTRA_LIBDEP is a pathname version of ABI_APPLIBDEP used for
##              checking dependencies in the final link.

ifeq ($(OS_NAME),WIN32)
EXTRA_LIBS	= 	$(addprefix $(LIBDIR)/lib,$(addsuffix $(ABI_VERSION)_s.lib,$(ABI_APPLIBS)))	\
			$(addsuffix .lib,$(ABI_LIBS))
EXTRA_LIBDEP	=	$(addprefix $(LIBDIR)/lib,$(addsuffix $(ABI_VERSION)_s.lib,$(ABI_APPLIBDEP)))
else
EXTRA_LIBS	=	-L$(LIBDIR) 							\
			$(addprefix -l,$(addsuffix $(ABI_VERSION),$(ABI_APPLIBS)))	\
			$(addprefix -l,$(ABI_LIBS))
EXTRA_LIBDEP	=	$(addprefix $(LIBDIR)/lib,$(addsuffix $(ABI_VERSION).a,$(ABI_APPLIBDEP)))
endif

##################################################################
##################################################################
## Generic Unix includes for Gtk, as it moves about installation paths.
## We should change this when get non-gtk versions on unix....

ifeq ($(ABI_NATIVE),unix)
ifdef ABI_OPT_GNOME
GNOME_CFLAGS	:=	$(shell $(GNOME_CONFIG) --cflags gnomeui)
GNOME_LIBS	:=	$(shell $(GNOME_CONFIG) --libs gnomeui)
CFLAGS 		+=	$(GNOME_CFLAGS) -DHAVE_GNOME
EXTRA_LIBS	+=	$(GNOME_LIBS)
ABI_GNOME_DIR		= gnome
ABI_GNOME_PREFIX	= Gnome
ABI_OPTIONS+=Gnome:On
else
GTK_CFLAGS	:=	$(shell $(GTK_CONFIG) --cflags)
GTK_LIBS	:=	$(shell $(GTK_CONFIG) --libs)
CFLAGS 		+=	$(GTK_CFLAGS)
EXTRA_LIBS	+=	$(GTK_LIBS)
ABI_OPTIONS+=Gnome:Off
endif
endif

##################################################################

