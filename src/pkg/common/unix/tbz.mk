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
## tbz -- this is a very simple tree of the essentials.  this is
##		just a tar of everything and can be put in /usr/local
##		and with a few symbolic links should work just fine....
##		the install script included with this package should
##		handle installation issues for all Unix platforms.

TBZ_PKGBASENAME_DYNAMIC	= $(PKGBASENAME)_dynamic
TBZ_PKGBASENAME_STATIC	= $(PKGBASENAME)_static

tbz_dynamic:
ifeq ($(UNIX_CAN_BUILD_DYNAMIC),1)
	@echo "Building .tar.bz2 package [dynamic] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(TBZ_PKGBASENAME_DYNAMIC),$(VERIFY_DIRECTORY))
	(cp ../common/unix/data/tbz_install.sh $(DIST)/$(TBZ_PKGBASENAME_DYNAMIC)/install.sh)
	(cd $(OUTDIR); tar cf - bin/*_d AbiSuite) | (cd $(DIST)/$(TBZ_PKGBASENAME_DYNAMIC); tar xf -)
	(cd $(DIST)/$(TBZ_PKGBASENAME_DYNAMIC); mv -f bin AbiSuite)
	(cd $(DIST)/$(TBZ_PKGBASENAME_DYNAMIC); strip AbiSuite/bin/*_d;)
	(cd $(DIST)/$(TBZ_PKGBASENAME_DYNAMIC)/AbiSuite; tar cf ../data.tar .)
	(cd $(DIST)/$(TBZ_PKGBASENAME_DYNAMIC); rm -rf AbiSuite)
	(cd $(DIST); tar cf - $(TBZ_PKGBASENAME_DYNAMIC) | bzip2 - - > $(TBZ_PKGBASENAME_DYNAMIC).tar.bz2)
	(cd $(DIST); rm -rf $(TBZ_PKGBASENAME_DYNAMIC))
endif

tbz_static:
ifeq ($(UNIX_CAN_BUILD_STATIC),1)
	@echo "Building .tar.bz2 package [static] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(TBZ_PKGBASENAME_STATIC),$(VERIFY_DIRECTORY))
	(cp ../common/unix/data/tbz_install.sh $(DIST)/$(TBZ_PKGBASENAME_STATIC)/install.sh)
	(cd $(OUTDIR); tar cf - bin/*_s AbiSuite) | (cd $(DIST)/$(TBZ_PKGBASENAME_STATIC); tar xf -)
	(cd $(DIST)/$(TBZ_PKGBASENAME_STATIC); mv -f bin AbiSuite)
	(cd $(DIST)/$(TBZ_PKGBASENAME_STATIC); strip AbiSuite/bin/*_s;)
	(cd $(DIST)/$(TBZ_PKGBASENAME_STATIC)/AbiSuite; tar cf ../data.tar .)
	(cd $(DIST)/$(TBZ_PKGBASENAME_STATIC); rm -rf AbiSuite)
	(cd $(DIST); tar cf - $(TBZ_PKGBASENAME_STATIC) | bzip2 - - > $(TBZ_PKGBASENAME_STATIC).tar.bz2)
	(cd $(DIST); rm -rf $(TBZ_PKGBASENAME_STATIC))
endif

tbz: tbz_dynamic tbz_static

