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

# deb fonts are handled by these rules
include debfonts/debfonts.mk

##################################################################
## deb -- a Debian package of AbiSuite applications.  the locations
##		of these packages follow default Debian 2.1 locations

# update this manually?  That sucks
RPM_SUITE_BUILD_COUNTER	= 1

# We don't do static binaries considering these have deps which will be
# met.
RPM_PKGBASENAME_DYNAMIC_NOFONTS	= abisuite-nofonts_$(ABI_BUILD_VERSION)-$(RPM_SUITE_BUILD_COUNTER)
RPM_PKGBASENAME_DYNAMIC_FONTS	= abisuite_$(ABI_BUILD_VERSION)-$(RPM_SUITE_BUILD_COUNTER)

ALIEN_LOC = $(shell which alien)

ifeq ($(ALIEN_LOC),)
deb::
	@echo ""
	@echo "* Skipping rpm packaging: [dpkg-deb] not found."
	@echo ""
else 
deb: deb_fonts deb_dynamic_nofonts deb_dynamic_fonts
endif

deb_dynamic_nofonts:
ifdef UNIX_CAN_BUILD_DYNAMIC
	@echo "* Building .rpm package [dynamic,nofonts] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
# create directories
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/DEBIAN,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/bin/X11,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/doc/abisuite,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/share/abisuite,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/lib/menu,$(VERIFY_DIRECTORY))
# copy standard dist stuff into shared space in the deb
	(cd $(OUTDIR)/AbiSuite; tar cf - .) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/share/abisuite; tar xf -)
# remove fonts from this archive
	(rm -rf $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/share/abisuite/fonts)
# turn control-template into control via sed magic
	(sed "s/__ABI_SUITE_VERSION__/$(ABI_BUILD_VERSION)-$(DEB_SUITE_BUILD_COUNTER)/" deb/control-template \
		| sed "s/__ABI_SUITE_ARCH__/$(OS_ARCH)/" \
		> $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/DEBIAN/control)
# copy other DEBIAN meta-control stuff
	(cd deb; tar cf - postinst postrm) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/DEBIAN; tar xf -)
# copy binaries
	(cd $(OUTDIR)/bin; tar cf - AbiWord_d) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/bin/X11; tar xf -)
# copy wrapper scripts
	(cd deb; tar cf - AbiWord) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/bin/X11; tar xf -)
# generate symlinks to wrapper scripts
	(cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/bin/X11; ln -sf AbiWord abiword)
# copy docs 
	(cd deb; tar cf - README copyright) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/doc/abisuite; tar xf -)
# copy debian menu stuff 
	(cd deb; tar cf - abiword) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/usr/lib/menu; tar xf -)
# generate MD5 sums for files after everything is in place
	(cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS); find . -type f -exec md5sum {} \;) | (sed "s/\.\/usr/usr/") | \
		(grep -v "DEBIAN" > $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_NOFONTS)/DEBIAN/md5sums)
# use alien to convert deb to rpm

	(cd $(DIST); dpkg-deb -b $(RPM_PKGBASENAME_DYNAMIC_NOFONTS) $(RPM_PKGBASENAME_DYNAMIC_NOFONTS).rpm)
	(cd $(DIST); rm -rf $(RPM_PKGBASENAME_DYNAMIC_NOFONTS))
endif

########################################################################

deb_dynamic_fonts:
ifdef UNIX_CAN_BUILD_DYNAMIC
	@echo "* Building .deb package [dynamic,fonts] ..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
# create directories
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/DEBIAN,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/bin/X11,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/doc/abisuite,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/share/abisuite,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/lib/menu,$(VERIFY_DIRECTORY))
# copy standard dist stuff into shared space in the deb
	(cd $(OUTDIR)/AbiSuite; tar cf - .) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/share/abisuite; tar xf -)
# massage fonts into the correct place
	(cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/share/abisuite; rm -rf fonts/scripts)
	(cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/share/abisuite; mv -f fonts/data/* fonts)	
	(cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/share/abisuite; rm -rf fonts/data)
#	(rm -rf $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/share/abisuite/fonts)
# turn control-template into control via sed magic
	(sed "s/__ABI_SUITE_VERSION__/$(ABI_BUILD_VERSION)-$(DEB_SUITE_BUILD_COUNTER)/" deb/control-template \
		| sed "s/__ABI_SUITE_ARCH__/$(OS_ARCH)/" \
		> $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/DEBIAN/control)
# copy other DEBIAN meta-control stuff
	(cd deb; tar cf - postinst postrm) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/DEBIAN; tar xf -)
# copy binaries
	(cd $(OUTDIR)/bin; tar cf - AbiWord_d) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/bin/X11; tar xf -)
# copy wrapper scripts
	(cd deb; tar cf - AbiWord) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/bin/X11; tar xf -)
# generate symlinks to wrapper scripts
	(cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/bin/X11; ln -sf AbiWord abiword)
# copy docs 
	(cd deb; tar cf - README copyright) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/doc/abisuite; tar xf -)
# copy debian menu stuff 
	(cd deb; tar cf - abiword) | (cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/usr/lib/menu; tar xf -)
# generate MD5 sums for files after everything is in place
	(cd $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS); find . -type f -exec md5sum {} \;) | (sed "s/\.\/usr/usr/") | \
		(grep -v "DEBIAN" > $(DIST)/$(RPM_PKGBASENAME_DYNAMIC_FONTS)/DEBIAN/md5sums)
# build deb package
	(cd $(DIST); dpkg-deb -b $(RPM_PKGBASENAME_DYNAMIC_FONTS) $(RPM_PKGBASENAME_DYNAMIC_FONTS).deb)
	(cd $(DIST); rm -rf $(RPM_PKGBASENAME_DYNAMIC_FONTS))
endif

