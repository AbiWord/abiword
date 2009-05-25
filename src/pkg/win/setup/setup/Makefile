#! gmake

## AbiSource Setup Kit
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

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs.mk

#The setup file name, include version in name.
PROGRAM:=		$(BINDIR)/abiword-setup-$(ABI_BUILD_VERSION_MAJOR).$(ABI_BUILD_VERSION_MINOR).$(ABI_BUILD_VERSION_MICRO).exe
PLUGINTOOLS:=		$(PLUGINDIR)/abiword-plugins-tools-$(ABI_BUILD_VERSION_MAJOR).$(ABI_BUILD_VERSION_MINOR).$(ABI_BUILD_VERSION_MICRO).exe
PLUGINIMPEXP:=		$(PLUGINDIR)/abiword-plugins-impexp-$(ABI_BUILD_VERSION_MAJOR).$(ABI_BUILD_VERSION_MINOR).$(ABI_BUILD_VERSION_MICRO).exe

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

# MinGW needs to escape NSIS's parameter switch, but not for Cygwin make (MSVC build)
# but only sometimes, sometimes it doesn't matter, sometimes using it fails, I'm so confused...
ifeq ($(OS_NAME),MINGW32)
NSIS_DEF = "//D"
NSIS_OPT = //V3 $(NSIS_DEF)MINGW32=1
else
NSIS_DEF = "/D"
NSIS_OPT =
endif

NSIS_SRC 	:= ./NSISv2/*
NSIS_PLUGIN_SRC := ./plugin*.nsi

NSIS_OPT += $(NSIS_DEF)VERSION=$(ABI_BUILD_VERSION) $(NSIS_DEF)VERSION_MAJOR=$(ABI_BUILD_VERSION_MAJOR)
NSIS_OPT += $(NSIS_DEF)VERSION_MINOR=$(ABI_BUILD_VERSION_MINOR) $(NSIS_DEF)VERSION_MICRO=$(ABI_BUILD_VERSION_MICRO)
#e.g. NSIS_CRTL_URL=http://abiword.pchasm.org/microsoft/ NSIS_CRTL_FILENAME=msvcr70.dll
#or   NSIS_CRTL_LOCAL=\MS\REDIST\ NSIS_CRTL_FILENAME=msvcrt.dll NSIS_CRTL_WIN95ONLY

ifdef NSIS_CRTL_LOCAL
NSIS_OPT += $(NSIS_DEF)OPT_CRTL_LOCAL=$(NSIS_CRTL_LOCAL)
endif
ifdef NSIS_CRTL_URL
NSIS_OPT += $(NSIS_DEF)OPT_CRTL_URL=$(NSIS_CRTL_URL)
endif
ifdef NSIS_CRTL_FILENAME
NSIS_OPT += $(NSIS_DEF)OPT_CRTL_FILENAME=$(NSIS_CRTL_FILENAME)
else
NSIS_OPT += $(NSIS_DEF)OPT_CRTL_FILENAME=msvcrt.dll
endif
ifeq ($(NSIS_CRTL_WIN95ONLY),1)
NSIS_OPT += $(NSIS_DEF)OPT_CRTL_WIN95ONLY
endif


$(PROGRAM): 
	@rm -f $(BINDIR)/*.ns*
	@$(ABICOPY) -r $(NSIS_SRC) $(BINDIR)
	@cd $(BINDIR); \
	$(NSIS_ROOT)/makensis$(EXE_SUFFIX) $(NSIS_OPT) ./AbiWord.nsi

$(PLUGINTOOLS):
	rm -f $(PLUGINDIR)/*.ns*
	@$(ABICOPY) $(NSIS_PLUGIN_SRC) $(PLUGINDIR)
	@cd $(PLUGINDIR); \
	$(NSIS_ROOT)/makensis$(EXE_SUFFIX) $(NSIS_OPT) ./pluginTools.nsi

$(PLUGINIMPEXP):
	@rm -f $(PLUGINDIR)/*.ns*
	@$(ABICOPY) $(NSIS_PLUGIN_SRC) $(PLUGINDIR)
	@cd $(PLUGINDIR); \
	$(NSIS_ROOT)/makensis$(EXE_SUFFIX) $(NSIS_OPT) ./pluginImportersExporters.nsi

_NSIS_:
	@if [ -d $(NSIS_ROOT) ]; then	\
		echo Using installed NSIS version found at $(NSIS_ROOT);	\
	else	if [ -d $(ABI_ROOT)/../nsis ]; then	\
			echo "Found nsis in peer directory";	\
			echo "$(ABI_ROOT)/../nsis/ ";	\
			echo "Building NSIS compiler from source, if you have" ;	\
			echo "problems building, please try a release build from";	\
			echo "http://nsis.sourceforge.net instead."; \
			$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../nsis;		\
			NSIS_ROOT = $(BINDIR);	\
		else									\
			echo "NSIS not found.  You can check it out of cvs by"; \
			echo "cvs -d :pserver:anoncvs:anoncvs@anoncvs.abisource.com:/cvsroot checkout nsis"; \
			echo "or preferably, download from http://nsis.sourceforge.net"; \
			echo "You will also need the md5sum and untgz NSIS plugins, and"; \
			echo "basic support for Albanian added to the Language files";	\
			echo "available in the NSIS archives, Abi's NSIS peer cvs module, or";	\
			echo "from http://abiword.pchasm.org/source/";	\
			exit 1;						\
		fi	\
	fi


_W32NATIVE_PLUGIN_:
	@if [ -d $(ABI_ROOT)/../abiword-plugins/wp/impexp/graphics/win32native ]; then	\
		echo Found win32native graphic file filter plugin;	\
		echo Building for use with Windows cut and paste support and importers;	\
		$(MAKE) -f Makefile -C $(ABI_ROOT)/../abiword-plugins/wp/impexp/graphics/win32native;	\
	else	\
		echo WARNING: win32native graphic file filter plugin NOT found;	\
		echo Please download the peer abiword-plugins directory \
		echo Document import and cut-n-paste will not support standard image formats!;	\
		exit 1;	\
	fi

ifeq ($(ABI_OPT_DEBUG),1)

disttools:: distribution
distimpexp:: distribution
distribution::
	@echo ABI_OPT_DEBUG is set.  Building a setup program in
	@echo debug mode is not allowed.
	@exit 1

else

distribution:: _NSIS_ _W32NATIVE_PLUGIN_ $(PROGRAM)
	@echo Copying $(PROGRAM) to $(DIST)
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(PROGRAM) $(DIST)

disttools:: _NSIS_ $(PLUGINTOOLS)
	@echo Copying $(PLUGINTOOLS) to $(DIST)
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(PLUGINTOOLS) $(DIST)

distimpexp:: _NSIS_ $(PLUGINIMPEXP)
	@echo Copying $(PLUGINIMPEXP) to $(DIST)
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(PLUGINIMPEXP) $(DIST)

endif
