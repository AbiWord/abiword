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

ABI_DEPTH=../..
include $(ABI_DEPTH)/config/abi_defs_wp.mk

# rpm fonts are handled by these rules
include rpmfonts/rpmfonts.mk

##################################################################
## rpm -- a Debian package of AbiSuite applications.  the locations
##		of these packages follow default Debian 2.1 locations

# update this manually?  That sucks
RPM_SUITE_BUILD_COUNTER	= 1

# We don't do static binaries considering these have deps which will be
# met.
RPM_PKGBASENAME_DYNAMIC_NOFONTS	= abisuite-apps-$(ABI_BUILD_VERSION)-$(RPM_SUITE_BUILD_COUNTER)
RPM_PKGBASENAME_DYNAMIC_FONTS	= abisuite-$(ABI_BUILD_VERSION)-$(RPM_SUITE_BUILD_COUNTER)

# this is so ugly
ALIEN 				= $(shell [ -x /usr/bin/alien ] && echo 1)
RPM				= $(shell [ -x /usr/bin/rpm ] && echo 1)
RPM_TOPDIR			= $(strip $(shell grep "topdir:" /etc/rpmrc | sed "s/topdir://"))

ifneq ($(RPM),)
ifneq ($(ALIEN),)
rpm: rpm_fonts rpm_dynamic_nofonts deb_dynamic_fonts
else
rpm::
	@echo ""
	@echo "* Skipping rpm packaging: [alien] not found."
	@echo ""
endif
else
rpm::
	@echo ""
	@echo "* Skipping rpm packaging: [rpm] not found."
	@echo ""
endif

rpm_dynamic_nofonts: deb_dynamic_nofonts
ifdef UNIX_CAN_BUILD_DYNAMIC
	@echo "* Building .rpm package [dynamic,nofonts] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	(cd $(DIST); alien -k --to-rpm $(DEB_PKGBASENAME_DYNAMIC_NOFONTS).deb)
	(mv $(RPM_TOPDIR)/RPMS/$(OS_ARCH)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS).$(OS_ARCH).rpm $(DIST))
endif

########################################################################

rpm_dynamic_fonts:
ifdef UNIX_CAN_BUILD_DYNAMIC
	@echo "* Building .rpm package [dynamic,fonts] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	(cd $(DIST); $(ALIEN) --to-rpm $(DEB_PKGBASENAME_DYNAMIC_FONTS).deb)
	(mv $(RPM_TOPDIR)/RPMS/$(OS_ARCH)/$(RPM_PKGBASENAME_DYNAMIC_FONTS).$(OS_ARCH).rpm $(DIST))
endif

