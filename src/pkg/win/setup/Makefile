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

ifeq ($(ABI_OPT_NSIS_20),1)
PROGRAM=		$(BINDIR)/setup_abiword.${ABI_BUILD_VERSION_MAJOR}-$(ABI_BUILD_VERSION_MINOR)-${ABI_BUILD_VERSION_MICRO}.exe
else
PROGRAM=		$(BINDIR)/setup_abiword.exe
endif

TARGETS=		$(PROGRAM)

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

ifeq ($(ABI_OPT_NSIS_20),1)
NSIS_SRC =  ./NSISv2/*.ns*
NSIS_OPT =  /DVERSION=$(ABI_BUILD_VERSION) /DVERSION_MAJOR=$(ABI_BUILD_VERSION_MAJOR)
NSIS_OPT += /DVERSION_MINOR=$(ABI_BUILD_VERSION_MINOR) /DVERSION_MICRO=$(ABI_BUILD_VERSION_MICRO)
#e.g. NSIS_CRTL_URL=http://abiword.pchasm.org/microsoft/ NSIS_CRTL_FILENAME=msvcr70.dll
#or   NSIS_CRTL_LOCAL=\MS\REDIST\ NSIS_CRTL_FILENAME=msvcrt.dll NSIS_CRTL_WIN95ONLY
ifdef NSIS_CRTL_LOCAL
NSIS_OPT += /DOPT_CRTL_LOCAL=$(NSIS_CRTL_LOCAL)
endif
ifdef NSIS_CRTL_URL
NSIS_OPT += /DOPT_CRTL_URL=$(NSIS_CRTL_URL)
endif
ifdef NSIS_CRTL_FILENAME
NSIS_OPT += /DOPT_CRTL_FILENAME=$(NSIS_CRTL_FILENAME)
else
NSIS_OPT += /DOPT_CRTL_FILENAME=msvcrt.dll
endif
ifdef NSIS_CRTL_WIN95ONLY
NSIS_OPT += /DOPT_CRTL_WIN95ONLY
endif
else
NSIS_SRC = ./AbiWord.nsi
NSIS_OPT = 
endif

ifeq ($(OS_NAME),MINGW32)
NSIS_OPT += //DMINGW32=1
endif

#ifneq ($(wildcard $(ABI_ROOT)/../abiword-plugins/wp/impexp/graphics/bmp),)
#ifeq ($(OS_NAME),MINGW32)
#NSIS_OPT += //DHAVEBMP=1
#else
#NSIS_OPT += /DHAVEBMP=1
#endif
#endif

$(PROGRAM): $(BINDIR)/makensis$(EXE_SUFFIX)
	@rm -f $(BINDIR)/*.ns*; \
	$(ABICOPY) $(NSIS_SRC) $(BINDIR)/; \
	cd $(BINDIR); \
	$(BINDIR)/makensis$(EXE_SUFFIX) $(NSIS_OPT) ./AbiWord.nsi

_NSIS_:
	@if [ -d $(ABI_ROOT)/../nsis ]; then			\
		echo Found nsis in peer directory;		\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../nsis;		\
	else									\
		echo nsis not found;						\
		echo TODO provide a nicer error message;			\
		echo TODO with instructions for obtaining it;			\
		exit 1;								\
	fi

_BMP_PLUGIN_:
	@if [ -d $(ABI_ROOT)/../abiword-plugins/wp/impexp/graphics/bmp ]; then	\
		echo Found BMP graphic file filter plugin;	\
		echo Building for use with Windows cut and paste support;	\
		$(MAKE) -f Makefile -C $(ABI_ROOT)/../abiword-plugins/wp/impexp/graphics/bmp;	\
	else	\
		echo WARNING: BMP graphic file filter plugin NOT found;	\
		echo Please download the peer abiword-plugins directory \
		echo Cut and paste support may not work for images without this plugin!;	\
	fi

_W32NATIVE_PLUGIN_:
	@if [ -d $(ABI_ROOT)/../abiword-plugins/wp/impexp/graphics/win32native ]; then	\
		echo Found win32native graphic file filter plugin;	\
		echo Building for use with Windows cut and paste support and importers;	\
		$(MAKE) -f Makefile -C $(ABI_ROOT)/../abiword-plugins/wp/impexp/graphics/win32native;	\
	else	\
		echo WARNING: win32native graphic file filter plugin NOT found;	\
		echo Please download the peer abiword-plugins directory \
		echo Document import and cut-n-paste won't support standard image formats!;	\
		exit 1;	\
	fi

ifeq ($(ABI_OPT_DEBUG),1)

distribution::
	@echo ABI_OPT_DEBUG is set.  Building a setup program in
	@echo debug mode is not allowed.
	exit 1

else

distribution:: _NSIS_ _W32NATIVE_PLUGIN_ $(TARGETS)
	@echo Copying $(PROGRAM) to $(DIST)
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(PROGRAM) $(DIST)

endif
