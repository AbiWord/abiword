#! gmake

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

# escape prefix for sed replacement
# can someone explain me why we should put so much "\" ?
esc_prefix:=$(shell echo $(prefix) | sed "s/\//\\\\\\\\\\//g")

##################################################################
## This makefile serves two purposes.  It exposes an "install" 
## target, wherein it installs AbiSuite applications.  The
## eventual layout of these files is $(prefix)/AbiSuite, except
## for symbolic links in $(prefix/bin.  /usr/local is a good value
##  to use for $(prefix), and it is the default.

# The plain "install" common to all Unix targets
install:
	@TARGET=$(prefix)/share/AbiSuite-2.0 BINDIR=$(prefix)/bin SRCDIR=$(OUTDIR) \
		SCRIPTDIR=$(ABI_ROOT)/src/pkg/common/unix/scripts \
		$(ABI_ROOT)/src/pkg/common/unix/scripts/install.sh 

########################################################################

# Native Solaris pkg format; first does an install, then packages,
# then uninstalls.
pkg: install
	@echo "Producing Solaris PKG..."
	@sed "s/__ABI_BUILD_VERSION__/$(ABI_BUILD_VERSION)/" pkg/pkginfo \
		| sed "s/__ABI_ARCH__/$(OS_ARCH)/" \
		| sed "s/__ABI_BUILD_PACKAGER__/$(ABI_BUILD_PACKAGER)/" \
		| sed "s/__prefix__/$(esc_prefix)/" \
		> $(DIST)/pkginfo
	@sed "s/@DISABLE_JPEG@/#/g" pkg/checkinstall > $(DIST)/checkinstall
	@echo "i pkginfo" > $(DIST)/Prototype
	@echo "i checkinstall=./checkinstall" >> $(DIST)/Prototype
	@(cd $(prefix); find share/AbiSuite-2.0 $(prefix)/bin/AbiWord $(prefix)/bin/abiword -print \
		| pkgproto >> $(DIST)/Prototype)
	@(cd $(DIST); pkgmk -o -b $(prefix) -d $(DIST) -f Prototype)
	@rm -f $(DIST)/Prototype
	@rm -f $(DIST)/pkginfo
	@rm -f $(DIST)/checkinstall
	@echo "Translate package to single file..."
	@pkgtrans -s $(DIST) $(DIST)/ABIword-$(ABI_BUILD_VERSION)-$(OS_ARCH)-local.pkg ABIword
	@echo "Compressing pkg into .gz format..."
	@gzip -9 $(DIST)/ABIword-$(ABI_BUILD_VERSION)-$(OS_ARCH)-local.pkg
	@rm -rf $(DIST)/ABIword
	@echo "Removing installation..."
	@rm -rf $(prefix)/share/AbiSuite-2.0
	@rm -f $(prefix)/bin/AbiWord
	@rm -f $(prefix)/bin/abiword

########################################################################

# Tgz target is provided by a common Unix makefile

include $(ABI_ROOT)/src/pkg/common/unix/tgz.mk
include $(ABI_ROOT)/src/pkg/common/unix/tbz.mk

########################################################################

# This target should build all packages 
# uhh, no it shouldn't. -ed
distribution: tgz pkg
