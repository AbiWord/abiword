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
VENDOR      = abisource
PRODUCT     = abiword-$(ABI_BUILD_VERSION)
VANDP       = $(VENDOR)/$(PRODUCT)

PROCESSOR	= $(shell uname -p)
DATE		= $(shell date +"%Y\/%m\/%d")
#TODO: Make these value dynamic
PKGSIZE		= 3M
PRODSIZE	= 6M

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
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/$(PROCESSOR)/usr,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/$(PROCESSOR)/usr/photon,$(VERIFY_DIRECTORY))
#Create the XML/QPM template for this version
	(sed 's/(VERSION)/$(ABI_BUILD_VERSION)/g; s/(DATE)/$(DATE)/g; s/(PKGSIZE)/$(PKGSIZE)/g; s/PRODSIZE/$(PRODSIZE)/g' scripts/template.qpm >/tmp/infile.qpm)
#Put the dictionary and readme things in the platform independant package /usr/local/
	(cd $(OUTDIR); pax -rw AbiSuite $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/usr/local )
#Install the MANIFEST files for the platform independant component
	(scripts/pkgmaker -b /tmp/infile.qpm  $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP) )
#Put the actual executables (striped of the _s) in the processor dependant dirs
	(cd $(OUTDIR); pax -rw -s/_s// bin/*_s $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/$(PROCESSOR)/usr/photon )
#Install the MANIFEST files for the x86 dependant component
	(scripts/pkgmaker -b /tmp/infile.qpm $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP)/$(PROCESSOR))
#Extract a copy of the readme and put it in the root of this package?
#	(cd $(DIST)/$(QPM_PKGBASENAME_DYNAMIC)/$(VANDP); find /usr -name "*readme*" -exec "cp -v {} ./" )
#Tar/gzip up the whole thing and call it a qpk
	(cd $(DIST)/$(QPM_PKGBASENAME_DYNAMIC); tar cf ../$(QPM_PKGBASENAME_DYNAMIC).tar .)
	(cd $(DIST); gzip $(QPM_PKGBASENAME_DYNAMIC).tar)
	(cd $(DIST); mv -f $(QPM_PKGBASENAME_DYNAMIC).tar.gz $(QPM_PKGBASENAME_DYNAMIC).qpk )
#Create the qpm file
	(cat scripts/manifest.top /tmp/infile.qpm scripts/manifest.bottom > $(DIST)/$(QPM_PKGBASENAME_DYNAMIC).qpm)
#Tar/gzip the qpk and qpm file and call it a qpr
	(cd $(DIST); tar cf $(QPM_PKGBASENAME_DYNAMIC).tar $(QPM_PKGBASENAME_DYNAMIC).qpm $(QPM_PKGBASENAME_DYNAMIC).qpk)
	(cd $(DIST); gzip $(QPM_PKGBASENAME_DYNAMIC).tar )
	(cd $(DIST); mv -f $(QPM_PKGBASENAME_DYNAMIC).tar.gz $(QPM_PKGBASENAME_DYNAMIC).qpr )
#Clear up any remaining goo
	(cd $(DIST); rm -rf $(QPM_PKGBASENAME_DYNAMIC) *.tar *.gz)
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
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/$(PROCESSOR)/usr,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/$(PROCESSOR)/usr/photon,$(VERIFY_DIRECTORY))
#Create the XML/QPM template for this version
	(sed 's/(VERSION)/$(ABI_BUILD_VERSION)/g; s/(DATE)/$(DATE)/g; s/(PKGSIZE)/$(PKGSIZE)/g; s/PRODSIZE/$(PRODSIZE)/g' scripts/template.qpm >/tmp/infile.qpm)
#Put the dictionary and readme things in the platform independant package /usr/local/
	(cd $(OUTDIR); pax -rw AbiSuite $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/usr/local )
#Install the MANIFEST files for the platform independant component
	(scripts/pkgmaker -b /tmp/infile.qpm  $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP) )
#Put the actual executables (striped of the _s) in the processor dependant dirs
#	(cd $(OUTDIR); pax -rw -s/_s// bin/*_s $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/$(PROCESSOR)/usr/photon )
	(cd $(OUTDIR); pax -rw bin/*_s $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/$(PROCESSOR)/usr/photon )
	(cp scripts/AbiWord.sh $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/$(PROCESSOR)/usr/photon/bin/AbiWord )
#Install the MANIFEST files for the x86 dependant component
	(scripts/pkgmaker -b /tmp/infile.qpm $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP)/$(PROCESSOR))
#Extract a copy of the readme and put it in the root of this package?
#	(cd $(DIST)/$(QPM_PKGBASENAME_STATIC)/$(VANDP); find /usr -name "*readme*" -exec "cp -v {} ./" )
#Tar/gzip up the whole thing and call it a qpk
	(cd $(DIST)/$(QPM_PKGBASENAME_STATIC); tar cf ../$(QPM_PKGBASENAME_STATIC).tar .)
	(cd $(DIST); gzip $(QPM_PKGBASENAME_STATIC).tar)
	(cd $(DIST); mv -f $(QPM_PKGBASENAME_STATIC).tar.gz $(QPM_PKGBASENAME_STATIC).qpk )
#Create the qpm file
	(cat scripts/manifest.top /tmp/infile.qpm scripts/manifest.bottom > $(DIST)/$(QPM_PKGBASENAME_STATIC).qpm)
#Create the index file, the qrm file and the INSTALL.txt file for the standalone repository
	(cd $(DIST); echo "$(QPM_PKGBASENAME_STATIC).qpm" > index; echo "$(QPM_PKGBASENAME_STATIC).qpk" >> index)
	(cd $(DIST); echo "$(QPM_PKGBASENAME_STATIC).qpm" > index; echo "$(QPM_PKGBASENAME_STATIC).qpk" >> index)
	(sed 's/(DATE)/$(DATE)/g;' scripts/repository.qrm >$(DIST)/repository.qrm)
	(cp scripts/INSTALL.txt $(DIST))
#Tar/gzip the qpk and qpm (with qrm, index and INSTALL file) and call it a qpr
	(cd $(DIST); tar cf $(QPM_PKGBASENAME_STATIC).tar $(QPM_PKGBASENAME_STATIC).* *.qrm index INSTALL.txt)
	(cd $(DIST); gzip $(QPM_PKGBASENAME_STATIC).tar )
	(cd $(DIST); mv -f $(QPM_PKGBASENAME_STATIC).tar.gz $(QPM_PKGBASENAME_STATIC).qpr )
#Clear up any remaining goo
	(cd $(DIST); rm -rf $(QPM_PKGBASENAME_STATIC) *.tar *.gz *.txt index)
endif

qpkg: qpkg_dynamic qpkg_static

