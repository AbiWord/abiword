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

# deb fonts are handled by these rules
include debfonts.mk

##################################################################
## deb -- a Debian package of AbiSuite applications.  the locations
##		of these packages follow default Debian 2.1 locations
##		not the 
##		the install script included with this package should
##		handle installation issues for all Unix platforms.

PKGBASENAME_DYNAMIC_NOFONTS	= $(PKGBASENAME)_dynamic_nofonts
PKGBASENAME_STATIC_NOFONTS	= $(PKGBASENAME)_static_nofonts
PKGBASENAME_DYNAMIC_FONTS	= $(PKGBASENAME)_dynamic_fonts
PKGBASENAME_STATIC_FONTS	= $(PKGBASENAME)_static_fonts

# the dirs from the src/[yourplatformstringhere] directory where
# everything got put after the compile
tgz_dirs=	bin AbiSuite

# one requirement of this target is the fonts package, then all the
# different combinations of binary packages
tgz:	tgz_fonts tgz_dynamic_nofonts tgz_dynamic_fonts tgz_static_nofonts tgz_static_fonts

tgz_dynamic_nofonts:
ifdef UNIX_CAN_BUILD_DYNAMIC
	@echo "* Building .tar.gz package [dynamic,nofonts] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(PKGBASENAME_DYNAMIC_NOFONTS),$(VERIFY_DIRECTORY))
	(cp ../common/unix/install.sh $(DIST)/$(PKGBASENAME_DYNAMIC_NOFONTS))
	(cd $(OUTDIR); tar cf - $(tgz_dirs)) | (cd $(DIST)/$(PKGBASENAME_DYNAMIC_NOFONTS); tar xf -)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_NOFONTS); mv -f bin AbiSuite)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_NOFONTS); rm -rf AbiSuite/bin/*_s)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_NOFONTS); rm -rf AbiSuite/fonts)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_NOFONTS); strip AbiSuite/bin/*_d;)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_NOFONTS)/AbiSuite; tar cf ../data.tar .)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_NOFONTS); rm -rf AbiSuite)
	(cd $(DIST); tar cf - $(PKGBASENAME_DYNAMIC_NOFONTS) | gzip - - > $(PKGBASENAME_DYNAMIC_NOFONTS).tar.gz)
	(cd $(DIST); rm -rf $(PKGBASENAME_DYNAMIC_NOFONTS))
endif

tgz_dynamic_fonts:
ifdef UNIX_CAN_BUILD_DYNAMIC
	@echo "* Building .tar.gz package [dynamic,fonts] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(PKGBASENAME_DYNAMIC_FONTS),$(VERIFY_DIRECTORY))
	(cp ../common/unix/install.sh $(DIST)/$(PKGBASENAME_DYNAMIC_FONTS))
	(cd $(OUTDIR); tar cf - $(tgz_dirs)) | (cd $(DIST)/$(PKGBASENAME_DYNAMIC_FONTS); tar xf -)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_FONTS); mv -f bin AbiSuite)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_FONTS); rm -rf AbiSuite/bin/*_s)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_FONTS); strip AbiSuite/bin/*_d;)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_FONTS)/AbiSuite; tar cf ../data.tar .)
	(cd $(DIST)/$(PKGBASENAME_DYNAMIC_FONTS); rm -rf AbiSuite)
	(cd $(DIST); tar cf - $(PKGBASENAME_DYNAMIC_FONTS) | gzip - - > $(PKGBASENAME_DYNAMIC_FONTS).tar.gz)
	(cd $(DIST); rm -rf $(PKGBASENAME_DYNAMIC_FONTS))
endif

tgz_static_nofonts:
ifdef UNIX_CAN_BUILD_STATIC
	@echo "* Building .tar.gz package [static,nofonts] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(PKGBASENAME_STATIC_NOFONTS),$(VERIFY_DIRECTORY))
	(cp ../common/unix/install.sh $(DIST)/$(PKGBASENAME_STATIC_NOFONTS))
	(cd $(OUTDIR); tar cf - $(tgz_dirs)) | (cd $(DIST)/$(PKGBASENAME_STATIC_NOFONTS); tar xf -)
	(cd $(DIST)/$(PKGBASENAME_STATIC_NOFONTS); mv -f bin AbiSuite)
	(cd $(DIST)/$(PKGBASENAME_STATIC_NOFONTS); rm -rf AbiSuite/bin/*_d)
	(cd $(DIST)/$(PKGBASENAME_STATIC_NOFONTS); rm -rf AbiSuite/fonts)
	(cd $(DIST)/$(PKGBASENAME_STATIC_NOFONTS); strip AbiSuite/bin/*_s;)
	(cd $(DIST)/$(PKGBASENAME_STATIC_NOFONTS)/AbiSuite; tar cf ../data.tar .)
	(cd $(DIST)/$(PKGBASENAME_STATIC_NOFONTS); rm -rf AbiSuite)
	(cd $(DIST); tar cf - $(PKGBASENAME_STATIC_NOFONTS) | gzip - - > $(PKGBASENAME_STATIC_NOFONTS).tar.gz)
	(cd $(DIST); rm -rf $(PKGBASENAME_STATIC_NOFONTS))
endif

tgz_static_fonts:
ifdef UNIX_CAN_BUILD_STATIC
	@echo "* Building .tar.gz package [static,fonts] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(PKGBASENAME_STATIC_FONTS),$(VERIFY_DIRECTORY))
	(cp ../common/unix/install.sh $(DIST)/$(PKGBASENAME_STATIC_FONTS))
	(cd $(OUTDIR); tar cf - $(tgz_dirs)) | (cd $(DIST)/$(PKGBASENAME_STATIC_FONTS); tar xf -)
	(cd $(DIST)/$(PKGBASENAME_STATIC_FONTS); mv -f bin AbiSuite)
	(cd $(DIST)/$(PKGBASENAME_STATIC_FONTS); rm -rf AbiSuite/bin/*_d)
	(cd $(DIST)/$(PKGBASENAME_STATIC_FONTS); strip AbiSuite/bin/*_s;)
	(cd $(DIST)/$(PKGBASENAME_STATIC_FONTS)/AbiSuite; tar cf ../data.tar .)
	(cd $(DIST)/$(PKGBASENAME_STATIC_FONTS); rm -rf AbiSuite)
	(cd $(DIST); tar cf - $(PKGBASENAME_STATIC_FONTS) | gzip - - > $(PKGBASENAME_STATIC_FONTS).tar.gz)
	(cd $(DIST); rm -rf $(PKGBASENAME_STATIC_FONTS))
endif