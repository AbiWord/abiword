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
#PROGRAM=		$(BINDIR)/setup_abiword.${ABI_BUILD_VERSION_MAJOR}-$(ABI_BUILD_VERSION_MINOR)-${ABI_BUILD_VERSION_MICRO}.exe
PROGRAM=		$(BINDIR)/setup_abiword.exe
else
PROGRAM=		$(BINDIR)/setup_abiword.exe
endif

TARGETS=		$(PROGRAM)

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

ifeq ($(ABI_OPT_NSIS_20),1)
NSIS_SRC = ./AbiWord2.nsi
NSIS_OPT =  /DVERSION=$(ABI_BUILD_VERSION) /DVERSION_MAJOR=$(ABI_BUILD_VERSION_MAJOR)
NSIS_OPT += /DVERSION_MINOR=$(ABI_BUILD_VERSION_MINOR) /DVERSION_MICRO=$(ABI_BUILD_VERSION_MICRO)
ifdef NSIS_CRTL_URL
#e.g. NSIS_CRTL_URL=http://abiword.pchasm.org/microsoft/ NSIS_CRTL_FILENAME=msvcr70.dll
NSIS_OPT += /DOPT_CRTL_URL=$(NSIS_CRTL_URL) /DOPT_CRTL_FILENAME=$(NSIS_CRTL_FILENAME)
endif
else
NSIS_SRC = ./AbiWord.nsi
NSIS_OPT = 
endif

ifeq ($(OS_NAME),MINGW32)
NSIS_OPT += //DMINGW32=1
endif

$(PROGRAM): $(BINDIR)/makensis$(EXE_SUFFIX)
	@rm -f $(BINDIR)/AbiWord.nsi; \
	 $(ABICOPY) $(NSIS_SRC) $(BINDIR)/AbiWord.nsi; \
	 cd $(BINDIR); \
	 $(BINDIR)/makensis$(EXE_SUFFIX) $(NSIS_OPT) ./AbiWord.nsi

ifeq ($(ABI_OPT_DEBUG),1)

distribution::
	@echo ABI_OPT_DEBUG is set.  Building a setup program in
	@echo debug mode is not allowed.
	exit 1

else

distribution:: $(TARGETS)
	@echo Copying $(PROGRAM) to $(DIST)
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(PROGRAM) $(DIST)

endif
