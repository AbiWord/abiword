#! gmake

## TF NOTE: This is sooo ugly ... but it is a start
## to getting a native Neutrino package built.

## AbiSource Applications
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

ABI_ROOT:=$(shell pwd)/../../..
include $(ABI_ROOT)/src/config/abi_defs_wp.mk
include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## qpkg -- this is a very simple tree of the essentials.  this is
##		just a package file containing all of the essentials that
## 		should just be unpacked into a package repository.

QPM_PKGBASENAME_DYNAMIC	= $(PKGBASENAME)_dynamic
QPM_PKGBASENAME_STATIC	= $(PKGBASENAME)_static
VENDOR      = "abisource"
PRODUCT     = "abiword"
VANDP       = $(VENDOR)/$(PRODUCT)
PROCESSOR	= $(shell uname -p)

qpkg_dynamic:
ifeq ($(QNX_CAN_BUILD_DYNAMIC),1)
	@echo "Building QSSL package [dynamic] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_DYNAMIC),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VENDOR),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/usr,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/usr/local,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/$(PROCESSOR),$(VERIFY_DIRECTORY))
#Include the installation script
#	(cp data/qpkg_install.sh $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/install.sh)
#Put the dictionary and readme things in the platform independant package /usr/local/
	(cd $(OUTDIR); pax -rw AbiSuite $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/usr/local )
#Put the actual executables (striped of the _s) in the processor dependant dirs
	(cd $(OUTDIR); pax -rw -s/_s// bin/*_s $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/$(PROCESSOR) )
#Install the MANIFEST files
	(scripts/pkgmaker $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP) )
#Extract a copy of the readme and put it in the root of this package?
	(cd $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP); find ./ -name "*readme*" -exec "cp -v {} ./" )
#Tar up the whole thing
	(cd $(DIST)/$(QPM_PKGBASENAME_DYNAMIC); tar cf ../$(QPM_PKGBASENAME_DYNAMIC).tar .)
	(cd $(DIST); gzip $(QPM_PKGBASENAME_DYNAMIC).tar)
#Clear up any remaining goo
	(cd $(DIST); rm -rf $(QPM_PKGBASENAME_DYNAMIC))
endif

qpkg_static:
ifeq ($(QNX_CAN_BUILD_STATIC),1)
	@echo "Building QSSL package [static] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_STATIC),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VENDOR),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/usr,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/usr/local,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/$(PROCESSOR),$(VERIFY_DIRECTORY))
#Include the installation script
#	(cp data/qpkg_install.sh $(DIST)/$(QPM_PKGBASENAME_STATIC)/install.sh)
#Put the dictionary and readme things in the platform independant package /usr/local/
	(cd $(OUTDIR); pax -rw AbiSuite $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/usr/local )
#Put the actual executables (striped of the _s) in the processor dependant dirs
	(cd $(OUTDIR); pax -rw -s/_s// bin/*_s $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/$(PROCESSOR) )
#Install the MANIFEST files
	(scripts/pkgmaker $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP) )
#Extract a copy of the readme and put it in the root of this package?
	(cd $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP); find ./ -name "*readme*" -exec "cp -v {} ./" )
#Tar up the whole thing
	(cd $(DIST)/$(QPM_PKGBASENAME_STATIC); tar cf ../$(QPM_PKGBASENAME_STATIC).tar .)
	(cd $(DIST); gzip $(QPM_PKGBASENAME_STATIC).tar)
#Clear up any remaining goo
	(cd $(DIST); rm -rf $(QPM_PKGBASENAME_STATIC))
endif

qpkg: qpkg_dynamic qpkg_static

