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

#### To build with libxml2 (aka gnome-xml version 2)
#### as opposed to expat as XML parser 
#### add the following line back to the
#### Makefile, add the variable to the make command line, or set
#### this variable as an environment variable.  A full recompile
#### must be done when switching the value of this variable.
####
#### NOTE: the Makefiles use 'ifdef' rather than 'ifeq' so setting
#### NOTE: this to **any** value will enable it.
####
#### NOTE: this is still experimental and require version 2.x of 
#### NOTE: libxml (aka gnome-xml). Get the latest version from
#### NOTE: http://xmlsoft.org/
####
#### ABI_OPT_LIBXML2=1
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

#### To build with the bidi-rectional support enabled add the following
#### line back to the Makefile, add the variable to the make command line
#### or set this variable as an environment variable. If you wish to
#### default to RTL direction of text, do the same with the second
#### variable
####
#### ABI_OPT_BIDI_ENABLED=1
####
#### ABI_OPT_BIDI_RTL_DOMINANT=1
####

#### To build with the JPEG support with libjpeg enabled add the following
#### line back to the Makefile, add the variable to the make command line
#### or set this variable as an environment variable. 
####
#### ABI_OPT_LIBJPEG=1
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
# Suck in any existing defines if they exist
# This is primarily designed for the CygWin Win32 build - all the
# calls to sed that are made for each definition of OS_NAME,
# OS_RELEASE, etc. are for some reason a performance killer.
# Allowing them to be predefined in a persistent makefile speeds
# up the build quite a bit.
PREDEF_FILE := $(shell eval "if test ! -f $(ABI_ROOT)/src/config/predefines.mk; then echo '.sample'; fi")
include $(ABI_ROOT)/src/config/predefines.mk$(PREDEF_FILE)


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
ifndef OS_NAME
OS_NAME		:= $(shell uname -s | sed "s/\//-/" | sed "s/_/-/" | sed "s/-.*//g")
endif
ifndef OS_RELEASE
OS_RELEASE	:= $(shell uname -r | sed "s/\//-/" | sed "s/[() ].*//g")
endif
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

ifeq ($(OS_NAME), WIN32)
ifndef CYGWIN_MAJOR_VERSION
CYGWIN_MAJOR_VERSION := $(shell echo $(OS_RELEASE) | cut -d . -f 1)
endif
ifndef CYGWIN_MINOR_VERSION
CYGWIN_MINOR_VERSION := $(shell echo $(OS_RELEASE) | cut -d . -f 2)
endif
ifndef CYGWIN_REVISION
CYGWIN_REVISION      := $(shell echo $(OS_RELEASE) | cut -d . -f 3)
endif
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

ifndef ABI_ESCAPE_QUOTES
 ABI_ESCAPE_QUOTES=NO
 ifeq ($(OS_NAME),WIN32)
  ifeq ($(CYGWIN_MAJOR_VERSION),1)
   ifeq ($(CYGWIN_MINOR_VERSION),1)
    OLD_CYGWIN := $(shell expr $(CYGWIN_REVISION) "<=" 2)
    ifeq ($(OLD_CYGWIN),1)
     ABI_ESCAPE_QUOTES=YES
    endif
   endif
  endif
 endif
endif

##################################################################
##################################################################
#### if it is Darwin, we suspect taht we have MacOS X, hence we 
#### build MacOS version using Carbon. Change later when we 
#### support Darwin running X and other varieties (like MacOS X
#### using Cocoa). <hfiguiere@teaser.fr>
ifeq ($(OS_NAME), Darwin)
OS_NAME = MACOSX
endif

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

ifeq ($(OS_NAME), WIN32)
 ifeq ($(OS_RELEASE), 4.0) 
  # HACK: for old B19 users
  define TRANSFORM_TO_DOS_PATH 
  sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g' 
  endef 
 else
CYGWIN_ROOT := $(shell cygpath -w / | sed 's|\\|/|g')
  ifeq ($(CYGWIN_MAJOR_VERSION),1)
   ifeq ($(CYGWIN_MINOR_VERSION),1)
    NEW_CYGWIN := $(shell expr $(CYGWIN_REVISION) ">=" 6)
    ifeq ($(NEW_CYGWIN),1)
     # Are we dealing with the root directory?
	 CYGWIN_ROOT_TEST := $(shell echo $(CYGWIN_ROOT) | grep [A-Z]:\\\\$)
	 ifneq ($(CYGWIN_ROOT_TEST), $(CYGWIN_ROOT))
	  # Nope, append another slash
	  CYGWIN_ROOT := $(shell echo $(CYGWIN_ROOT) / | sed 's| ||g')
	 endif
    endif
   endif
  endif
  ifneq (,$(findstring  ,$(shell uname -r)))
   define TRANSFORM_TO_DOS_PATH
   sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g'
   endef
  else
   ifneq (,$(findstring cygdrive,$(ABI_ROOT)))
    define TRANSFORM_TO_DOS_PATH
    sed 's|/cygdrive/\([a-zA-Z]\)/|\1:/|g' | sed 's|/|\\\\|g'
    endef
   else
    define TRANSFORM_TO_DOS_PATH
    sed 's|/|$(CYGWIN_ROOT)|' | sed 's| /| $(CYGWIN_ROOT)|g' | sed 's|/|\\\\|g'
    endef
   endif
  endif
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
		/af/gr/xp		/af/gr/$(ABI_NATIVE) \
	        /wp/ap/xp \
                /wp/ap/$(ABI_NATIVE) \
                /text/ptbl/xp \
                /text/fmt/xp \
                /wp/impexp/xp \
                /wp/ap/xp/ToolbarIcons

ifeq ($(ABI_OPT_GNOME),1)
ABI_OPT_GNOMEVFS := 1
ABI_XAP_INCS+=	/af/xap/$(ABI_NATIVE)/$(ABI_GNOME_DIR)	\
		/af/ev/$(ABI_NATIVE)/$(ABI_GNOME_DIR) 
endif

# consider adding some UNIX native includes because MacOS X is really hybrid.
ifeq ($(OS_NAME), MACOSX)
ABI_XAP_INCS+= /af/util/unix
endif 

ABI_OTH_INCS=	/other/spell/xp \
                /other/fribidi/xp
ifeq ($(OS_NAME), WIN32)
ABI_OTH_INCS+=	/../../wv/glib-wv 
endif

ifeq ($(ABI_OPT_LIBXML2),1)
ABI_PEER_INCS=
else
ABI_PEER_INCS=	/../../expat/lib
endif
ABI_PEER_INCS+=/../../wv/exporter

# Test for iconv in system locations
HAVE_ICONV_SYSTEM := $(shell if [ -r /usr/include/iconv.h -o -r /usr/local/include/iconv.h ] ; then echo 1 ; fi)

ifeq ($(OS_NAME), WIN32)
ABI_PEER_INCS+=/../../libiconv/include
else
ifneq ($(HAVE_ICONV_SYSTEM),1)
ABI_PEER_INCS+=/../../libiconv/include
endif
endif

ABI_ALL_INCS=	$(ABI_XAP_INCS) $(ABI_PEER_INCS) $(ABI_AP_INCS) $(ABI_OTH_INCS) $(ABI_TM_INCS)

ifeq ($(OS_NAME), WIN32)
ABI_XX_ROOT:=$(shell echo $(ABI_ROOT) | $(TRANSFORM_TO_DOS_PATH) | sed 's|\\\\|/|g')
ABI_INCS=	$(addprefix -I$(ABI_XX_ROOT)/src,$(ABI_ALL_INCS))
else
ABI_XX_ROOT:=$(ABI_ROOT)
ABI_INCS=	$(addprefix -I$(ABI_ROOT)/src,$(ABI_ALL_INCS))
endif

##################################################################
##################################################################

## ABI_OPTIONS is a list of all the conditionally included options
##             suitable for echoing during the build process or
##             including in an AboutBox.

ABI_ENABLED_OPTIONS=

## conditionally enable some additional debugging and test code

ifeq ($(ABI_OPT_DEBUG),1)
ABI_DBGDEFS=		-DUT_DEBUG -DPT_TEST -DFMT_TEST -DUT_TEST
ABI_OPTIONS+=Debug:On
else
ABI_DBGDEFS=		-DNDEBUG
ABI_OPTIONS+=Debug:Off
endif

## BIDI options

ifeq ($(ABI_OPT_BIDI_ENABLED),1)
ABI_BIDI_ENABLED=-DBIDI_ENABLED
ABI_OPTIONS+=BiDi:On
ifeq ($(ABI_OPT_BIDI_RTL_DOMINANT),1)
ABI_BIDI_ENABLED+=-DBIDI_RTL_DOMINANT
ABI_OPTIONS+=/RTL dominant/
else
ABI_OPTIONS+=/LTR dominant/
endif
else
ABI_OPTIONS+=BiDi:Off
endif

ifeq ($(ABI_OPT_LIBJPEG),1)
LIBJPEG_CFLAGS=-DHAVE_LIBJPEG
ifeq ($(OS_NAME),Win32)
LIBJPEG_LIBS=libjpeg.lib
else
LIBJPEG_LIBS=-ljpeg
endif
ABI_OPTIONS+=libjpeg:On
else
LIBJPEG_CFLAGS=
LIBJPEG_LIBS=
endif

##################################################################
##################################################################

LINK_DLL	= $(LINK) $(OS_DLLFLAGS) $(DLLFLAGS)

CFLAGS		= $(OPTIMIZER) $(WARNFLAGS) $(OS_CFLAGS) $(DEFINES) $(INCLUDES) $(OS_INCLUDES) $(XCFLAGS)	\
			$(ABI_TMDEFS) $(ABI_NAMEDEFS) $(ABI_APPLIBDIRDEF)	\
			$(ABI_DBGDEFS) $(ABI_BIDI_ENABLED) $(ABI_INCS) $(LIBJPEG_CFLAGS)

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
ifeq ($(OS_NAME), QNX)
ifeq (,$(suffix $(OS_RELEASE))) 
# QNX 4 not supported
else
include $(ABI_ROOT)/src/config/platforms/nto.mk
endif
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

ifeq ($(OS_NAME), MACOSX)
include $(ABI_ROOT)/src/config/platforms/macosx.mk
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
#################################################################
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
## ABI_BUILD_VERSION	should be set to the build version (1.0.3)
##			for a numbered build.
##
## ABI_BUILD_ID		can be used as a identifying label (such as
##			a date stamp in a nightly build system).
##

ABI_BUILD_VERSION	= 1.0.3
ABI_BUILD_ID		=

CFLAGS  += -DABI_BUILD_VERSION=\"$(ABI_BUILD_VERSION)\"

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
PLUGINDIR       = $(OUTDIR)/plugins

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
			$(addsuffix .lib,$(ABI_LIBS)) $(LIBJPEG_LIBS)
EXTRA_LIBDEP	=	$(addprefix $(LIBDIR)/lib,$(addsuffix $(ABI_VERSION)_s.lib,$(ABI_APPLIBDEP)))
else
EXTRA_LIBS	=	-L$(LIBDIR) 							\
			$(addprefix -l,$(addsuffix $(ABI_VERSION),$(ABI_APPLIBS)))	\
			$(addprefix -l,$(ABI_LIBS)) $(LIBJPEG_LIBS)
EXTRA_LIBDEP	=	$(addprefix $(LIBDIR)/lib,$(addsuffix $(ABI_VERSION).a,$(ABI_APPLIBDEP)))
endif

##################################################################
##################################################################
## Generic Unix includes for Gtk, as it moves about installation paths.
## We should change this when get non-gtk versions on unix....

ifeq ($(ABI_NATIVE),unix)

# ABI_OPT_BONOBO turns on ABI_OPT_GNOME
# But we should build without gnome by default
ABI_OPT_GNOME = 0
ifeq ($(ABI_OPT_BONOBO),1)
	ABI_OPT_GNOME = 1
endif

# ABI_OPT_GNOME_DIRECT_PRINT enables "Print directly" command for
# gnome port
# require bonobo for the gnome port too
#ABI_OPT_GNOME_DIRECT_PRINT = 1
ifeq ($(ABI_OPT_GNOME),1)
	ABI_OPT_GNOME_DIRECT_PRINT = 1
	ABI_OPT_BONOBO = 1
endif

# This next bit is ugly so I'll explain my rationale:
# gnome-config --libs links us against lots of libs that we don't need.
# This is usually harmless, esp. if you build from source. But say
# that I build a binary and gnome-config --libs returns -lgnomefont.
# We don't require gnome-font. Most people don't have gnome-font
# installed. So when they go to run Abi, it complains about not
# finding the proper library to link against. This sucks. This is my
# work-around. It isn't the greatest but it's the best solution that my
# puny intellect could come up with, at least in the short-term ;^)

ifeq ($(ABI_OPT_GNOME),1)
GNOME_CFLAGS	:= $(shell $(GNOME_CONFIG) --cflags gnomeui gal print gdk_pixbuf)
GNOME_LIBS      := $(shell $(GTK_CONFIG) --libs)
# These also might be needed: -lSM -lICE
GTK_CFLAGS      := $(shell $(GTK_CONFIG) --cflags)
GNOME_CFLAGS += $(GTK_CFLAGS)
GNOME_LIBS      += $(shell $(GNOME_CONFIG) --libs-only-L gnome gal gdk_pixbuf)
GNOME_LIBS      += -lgnomeui -lgnomeprint -lgal -lart_lgpl -lgdk_imlib -lgnome -lgnomesupport -lxml -lglade-gnome -lglade -lgnomecanvaspixbuf -lgdk_pixbuf -ltiff -ljpeg 
#
# Enable this line for electric fence.
#
#GNOME_LIBS	+= -lefence

ifeq ($(ABI_OPT_GNOMEVFS),1)
GNOME_CFLAGS += $(shell gnome-vfs-config --cflags)
GNOME_CFLAGS += -DHAVE_GNOMEVFS
GNOME_LIBS   += $(shell gnome-vfs-config --libs)
ABI_OPTIONS  +=GnomeVFS:On
endif

GNOME_CFLAGS += $(shell $(GLIB_CONFIG) --cflags)

# the bonobo target is known not to work properly yet
ifeq ($(ABI_OPT_BONOBO),1)
GNOME_CFLAGS    += $(shell $(GNOME_CONFIG) --cflags oaf bonobo)
GNOME_CFLAGS    += -DHAVE_BONOBO
GNOME_LIBS      += -lbonobo -loaf -lORBitCosNaming -lORBit -lIIOP -lORBitutil $(shell $(GNOME_CONFIG) --libs bonobox) -lbonobo-print
ABI_OPTIONS+=Bonobo:On
else
ABI_OPTIONS+=Bonobo:Off
endif

#ifeq ($(ABI_OPT_GNOME_DIRECT_PRINT),1)
GNOME_CFLAGS    +=	-DHAVE_GNOME_DIRECT_PRINT
ABI_OPTIONS+=DirectPrint:On
#else
ABI_OPTIONS+=DirectPrint:Off
#endif

CFLAGS 		+=	$(GNOME_CFLAGS) -DHAVE_GNOME
EXTRA_LIBS	+=	$(GNOME_LIBS)
ABI_GNOME_DIR		= gnome
ABI_GNOME_PREFIX	= Gnome
ABI_OPTIONS+=Gnome:On

else
# vanilla gtk port
ABI_OPT_GNOME=
GTK_CFLAGS	:=	$(shell $(GTK_CONFIG) --cflags)
GTK_LIBS	:=	$(shell $(GTK_CONFIG) --libs)
GLIB_CFLAGS     :=      $(shell $(GLIB_CONFIG) --cflags)
CFLAGS 		+=	$(GTK_CFLAGS) $(GLIB_CFLAGS)
EXTRA_LIBS	+=	$(GTK_LIBS)
#
# Enable this line for electric fence.
#
#EXTRA_LIBS	+=	$(GTK_LIBS) -lefence
ABI_OPTIONS+=Gnome:Off
endif

ifeq ($(ABI_OPT_LIBXML2),1)
XML_CFLAGS	= $(shell $(LIBXML_CONFIG) --cflags)
XML_LIBS	= $(shell $(LIBXML_CONFIG) --libs)
CFLAGS 		+=	$(XML_CFLAGS) -DHAVE_LIBXML2
EXTRA_LIBS	+=	$(XML_LIBS)
ABI_OPTIONS+=LibXML:On
else
ABI_OPTIONS+=LibXML:Off
endif
endif

ifeq ($(OS_NAME), WIN32)
EXTRA_LIBS	+= $(ABI_ROOT)/../psiconv/psiconv/.libs/libpsiconv.lib
EXTRA_LIBS	+= $(ABI_ROOT)/../expat/lib/.libs/libexpat.lib
LDFLAGS += /NODEFAULTLIB:LIBC
else
# For psiconv. This should pick up the static psiconv library in the
# peer directory, or the global one if it is not found. 
EXTRA_LIBS	+= -L$(ABI_ROOT)/../psiconv/psiconv/.libs -lpsiconv
ifneq ($(ABI_OPT_LIBXML2),1)
EXTRA_LIBS	+= -L$(ABI_ROOT)/../expat/lib/.libs -lexpat 
endif
endif

##################################################################
##################################################################
## Pspell spell checker
## Should only be enabled for unix or else do the
## addprefix addsuffix stuff as shown above
## Need ltdl for dynamic loading/linking on Solaris
ifeq ($(ABI_OPT_PSPELL),1)
EXTRA_LIBS      += -lpspell -lpspell-modules -lltdl
ABI_OPTIONS+=Pspell:On
CFLAGS += -DHAVE_PSPELL
else
ABI_OPTIONS+=Pspell:Off
endif

# Perl scripting support
ifeq ($(ABI_OPT_PERL),1)
    ifeq ($(OS_NAME), WIN32)
		EXTRA_LIBS += $(shell perl -MExtUtils::Embed -e ldopts | sed 's|[a-zA-Z0-9~:\\]*msvcrt\.lib||g' | sed 's|\([-/]release\|[-/]nodefaultlib\|[-/]nologo\|[-/]machine:[a-zA-Z0-9]*\)||g' | sed 's|\\|\\\\\\\\|g')
		CFLAGS += -DABI_OPT_PERL $(shell perl -MExtUtils::Embed -e ccopts | sed 's/\(-O1\|-MD\)//g')
    else
		EXTRA_LIBS += $(shell perl -MExtUtils::Embed -e ldopts)
		CFLAGS += -DABI_OPT_PERL $(shell perl -MExtUtils::Embed -e ccopts) -Ubool
    endif
	ABI_OPTIONS+=Scripting:On
else
	ABI_OPTIONS+=Scripting:Off
endif

# conditionally enable stl-based implementations of our
# UT_XXX classes. We may need to link against certain
# libraries, but we don't on linux. Add these as necessary.
ifeq ($(ABI_OPT_STL),1)
CFLAGS += -DABI_OPT_STL
ABI_OPTIONS+=STL:On
endif

ifeq ($(ABI_OPT_WIDGET),1)
CFLAGS += -DABI_OPT_WIDGET
endif

ifeq ($(ABI_OPT_PLUGINS),1)
CFLAGS += -DENABLE_PLUGINS
ABI_OPTIONS+=Plugins:On
endif

ifeq ($(ABI_USE_100_ISPELL),1)
CFLAGS += -DMAXSTRINGCHARS=100
endif


# 
# yep, this is an egregiously ugly place to hardwire this, but 
# it's the easiest way to ensure that we always include iconv.h 
# with it set (to prevent linker mismatches with wv's version)
#  
# fjf: I'm commenting it. wv should be using the same iconv as
#      AbiWord, and both should be using libiconv-1.7 or equiv.
#      LIBICONV_PLUG should be set *only* if building against
#      peer libiconv
#  
# CFLAGS += -DLIBICONV_PLUG

ifeq ($(ABI_NATIVE),unix)
CFLAGS += -DSUPPORTS_UT_IDLE=1
endif

ifeq ($(ABI_OPT_BIDI_ENABLED),1)
	ifeq ($(OS_NAME), WIN32)
		EXTRA_LIBS += $(LIBDIR)/libAbi_fribidi_s.lib
	else
		EXTRA_LIBS += -lAbi_fribidi
	endif
endif

