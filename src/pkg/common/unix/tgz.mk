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

##################################################################
## tgz -- this is a very simple tree of the essentials.  this is
##		just a tar of everything and can be put in /usr/local
##		and with a few symbolic links should work just fine....
##		the install script included with this package should
##		handle installation issues for all Unix platforms.

TGZ_PKGBASENAME_DYNAMIC	= $(PKGBASENAME)_dynamic
TGZ_PKGBASENAME_STATIC	= $(PKGBASENAME)_static

tgz_dynamic:
ifeq ($(UNIX_CAN_BUILD_DYNAMIC),1)
	@echo "Building .tar.gz package [dynamic] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(TGZ_PKGBASENAME_DYNAMIC),$(VERIFY_DIRECTORY))
	(cp ../common/unix/data/tgz_install.sh $(DIST)/$(TGZ_PKGBASENAME_DYNAMIC)/install.sh)
	(cd $(OUTDIR); tar cf - bin/*_d AbiSuite) | (cd $(DIST)/$(TGZ_PKGBASENAME_DYNAMIC); tar xf -)
	(cd $(DIST)/$(TGZ_PKGBASENAME_DYNAMIC); mv -f bin AbiSuite)
	(cd $(DIST)/$(TGZ_PKGBASENAME_DYNAMIC); strip AbiSuite/bin/*_d;)
	(cd $(DIST)/$(TGZ_PKGBASENAME_DYNAMIC)/AbiSuite; tar cf ../data.tar .)
	(cd $(DIST)/$(TGZ_PKGBASENAME_DYNAMIC); rm -rf AbiSuite)
	(cd $(DIST); tar cf - $(TGZ_PKGBASENAME_DYNAMIC) | gzip - - > $(TGZ_PKGBASENAME_DYNAMIC).tar.gz)
	(cd $(DIST); rm -rf $(TGZ_PKGBASENAME_DYNAMIC))
endif

tgz_static:
ifeq ($(UNIX_CAN_BUILD_STATIC),1)
	@echo "Building .tar.gz package [static] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(TGZ_PKGBASENAME_STATIC),$(VERIFY_DIRECTORY))
	(cp ../common/unix/data/tgz_install.sh $(DIST)/$(TGZ_PKGBASENAME_STATIC)/install.sh)
	(cd $(OUTDIR); tar cf - bin/*_s AbiSuite) | (cd $(DIST)/$(TGZ_PKGBASENAME_STATIC); tar xf -)
	(cd $(DIST)/$(TGZ_PKGBASENAME_STATIC); mv -f bin AbiSuite)
	(cd $(DIST)/$(TGZ_PKGBASENAME_STATIC); strip AbiSuite/bin/*_s;)
	(cd $(DIST)/$(TGZ_PKGBASENAME_STATIC)/AbiSuite; tar cf ../data.tar .)
	(cd $(DIST)/$(TGZ_PKGBASENAME_STATIC); rm -rf AbiSuite)
	(cd $(DIST); tar cf - $(TGZ_PKGBASENAME_STATIC) | gzip - - > $(TGZ_PKGBASENAME_STATIC).tar.gz)
	(cd $(DIST); rm -rf $(TGZ_PKGBASENAME_STATIC))
endif

tgz: tgz_dynamic tgz_static

