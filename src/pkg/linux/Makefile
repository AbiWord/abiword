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
## This makefile serves two purposes.  It exposes an "install" 
## target, wherein it installs AbiSuite applications.  The
## eventual layout of these files is $(prefix)/AbiSuite, except
## for symbolic links in $(prefix/bin.  /usr/local is a good value
##  to use for $(prefix), and it is the default.
## 
## If you're looking to build distribution packages, this Makefile
## also provides the "distribution" target, which depends on 
## installation targets specific to the distribution layouts
## it will build.  These targets might be named "install_redhat"
## or "install_debian"; they place the program components into places
## their respective distributions would expect to find them.  These
## targets will be called by package tools like RPM.

# The plain "install" common to all Unix targets
install:
	@TARGET=$(prefix)/AbiSuite BINDIR=$(prefix)/bin SRCDIR=$(OUTDIR) \
		SCRIPTDIR=$(ABI_ROOT)/src/pkg/common/unix/scripts \
		$(ABI_ROOT)/src/pkg/common/unix/scripts/install.sh 

########################################################################

install_redhat:
	@strip $(OUTDIR)/bin/AbiWord_d $(OUTDIR)/bin/AbiWord_s
	@TARGET=/usr/share/abisuite BINDIR=/usr/bin SRCDIR=$(OUTDIR) \
		DOCDIR=/usr/doc LIBDIR=/usr/lib \
		SCRIPTDIR=$(ABI_ROOT)/src/pkg/common/unix/scripts \
		$(ABI_ROOT)/src/pkg/linux/scripts/install_redhat.sh 

########################################################################

RPM_LOC = $(shell [ -x /usr/bin/rpm ] && echo 1 || [ -x /bin/rpm ] && echo 1)
ALIEN_LOC = $(shell [ -x /usr/bin/alien ] && echo 1)

# If tgz is inside $ABI_DIST_TARGET
ifneq ($(findstring tgz,$(ABI_DIST_TARGET)),)
	DIST_DEPS += tgz
endif

# If tbz is inside $ABI_DIST_TARGET
ifneq ($(findstring tbz,$(ABI_DIST_TARGET)),)
	DIST_DEPS += tbz
endif

# If rpm is inside $ABI_DIST_TARGET _and_ rpm is found locally
ifneq ($(findstring rpm,$(ABI_DIST_TARGET)),)
ifneq ($(RPM_LOC),)
	DIST_DEPS += rpm
else
bogus1:
	@echo ""
	@echo "Skipping RPM packaging: [/usr/bin/rpm] not found."
	@echo ""
endif
endif

# If deb is inside $ABI_DIST_TARGET _and_ alien is found locally
ifneq ($(findstring deb,$(ABI_DIST_TARGET)),)
#ifneq ($(ALIEN_LOC),)
	DIST_DEPS += deb
#else
#bogus2:
#	@echo ""
#	@echo "Skipping DEB packaging: [/usr/bin/alien] not found."
#	@echo ""
#endif
endif

# If slp is inside $ABI_DIST_TARGET _and_ alien is found locally
ifneq ($(findstring slp,$(ABI_DIST_TARGET)),)
ifneq ($(ALIEN_LOC),)
	DIST_DEPS += slp
else
bogus3:
	@echo ""
	@echo "Skipping SLP packaging: [/usr/bin/alien] not found."
	@echo ""
endif
endif

# If rpm was NOT in the dist target list, but deb _or_ slp was,
# we wipe the RPM so it will not conflict with the RPM generated
# by another machine (which was explicitly told to create one).

ifeq ($(findstring rpm,$(ABI_DIST_TARGET)),)
ifneq ($(findstring slp,$(ABI_DIST_TARGET)),)
	WIPE_RPM=1
endif
endif

########################################################################

# This target builds a huge tarball of EVERY component from CVS needed
# to build the AbiSuite tree.

#DEBIAN_ROOT is the root of all work being done for debian build
#DEB_ROOT is the root of all work being done for just the deb archive
DEBIAN_ROOT:=$(ABI_ROOT)/dist/abisuite-$(ABI_BUILD_VERSION)
DEB_ROOT:=$(ABI_ROOT)/dist/abisuite-$(ABI_BUILD_VERSION)/debian/tmp



PACK_ROOT:=$(ABI_ROOT)/../abisuite-$(ABI_BUILD_VERSION)

MODULE_LIST=abi-$(ABI_BUILD_VERSION) expat unixfonts abidistfiles libpng zlib wv

# The following only works for RPM3.x
#RPM_SOURCE_DIR=$(strip $(shell grep "topdir:" /etc/rpmrc | sed "s/.*topdir://"))

# This should work for RPM 3.x and RPM 4.x on stock redhat systems
RPM_SOURCE_DIR=/usr/src/redhat

# TODO : should this be specified outside the build process?  
ABI_RELEASE_NUMBER=1

pack_rpm:
	@rm -rf $(PACK_ROOT)
	@echo "Making source pack in directory [$(PACK_ROOT)]..."
	@mkdir -p $(PACK_ROOT)
	@(cd $(ABI_ROOT)/..; tar --exclude=CVS -cf - $(MODULE_LIST)) | \
		(cd $(PACK_ROOT); tar xf -)
#	@rm -f $(RPM_SOURCE_DIR)/SOURCES/abisuite-$(ABI_BUILD_VERSION).tar
	@(cd $(PACK_ROOT)/..; tar cf $(RPM_SOURCE_DIR)/SOURCES/abisuite-$(ABI_BUILD_VERSION).tar \
		 abisuite-$(ABI_BUILD_VERSION))
	@rm -rf $(PACK_ROOT)

########################################################################

# These targets actually do all the heavy lifting of packaging.

rpm: pack_rpm
# A tarball is already sitting in /usr/src/redhat/SOURCES (or thereabouts)
# so rip the spec template through some sed magic and run with it.
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))	
	@echo "Exporting spec file to [$(RPM_SOURCE_DIR)/SPECS]..."
	@(sed "s/__ABI_BUILD_VERSION__/$(ABI_BUILD_VERSION)/" rpm/abisuite.spec-template \
		| sed "s/__ABI_RELEASE_NUMBER__/$(ABI_RELEASE_NUMBER)/" \
		> $(RPM_SOURCE_DIR)/SPECS/abisuite-$(ABI_BUILD_VERSION).spec)
	@echo "Producing RPM..."
	rpm -ba $(RPM_SOURCE_DIR)/SPECS/abisuite-$(ABI_BUILD_VERSION).spec
	mv $(RPM_SOURCE_DIR)/RPMS/$(OS_ARCH)/abisuite-$(ABI_BUILD_VERSION)-$(ABI_RELEASE_NUMBER).$(OS_ARCH).rpm $(DIST)
	mv $(RPM_SOURCE_DIR)/SRPMS/abisuite-$(ABI_BUILD_VERSION)-$(ABI_RELEASE_NUMBER).$(OS_ARCH).src.rpm $(DIST)

deb::
	@echo "Producing Deb..."
	@rm -rf $(DEBIAN_ROOT)
	@echo "Building Tree in directory [$(DEBIAN_ROOT)]..."
#sources are put in /usr/bin because /usr/X11R6/bin is reserved for
#programs that are a part of the X distribution.  FHS-compliance
	@install -m 755 -d $(DEB_ROOT)/DEBIAN
	@install -m 755 -d $(DEB_ROOT)/usr/bin
	@install -m 755 -d $(DEB_ROOT)/usr/share/doc/abiword/examples
	@install -m 755 -d $(DEB_ROOT)/usr/share/abisuite/fonts
	@install -m 755 -d $(DEB_ROOT)/usr/share/abisuite/dictionary
	@install -m 755 -d $(DEB_ROOT)/usr/share/abisuite/icons
	@install -m 755 -d $(DEB_ROOT)/usr/share/abisuite/AbiWord/strings
	@install -m 755 -d $(DEB_ROOT)/usr/share/abisuite/AbiWord/help
	@install -m 755 -d $(DEB_ROOT)/usr/lib/menu
	@install -m 755 -d $(DEB_ROOT)/usr/share/man/man1
	@chmod -R 755 $(DEB_ROOT)/.
#install binaries
	@install -s -m 755 $(OUTDIR)/bin/*_d $(DEB_ROOT)/usr/bin/.
	@install -m 755 deb/AbiWord $(DEB_ROOT)/usr/bin/.
	@ln -s AbiWord $(DEB_ROOT)/usr/bin/abiword
#install the documentation
	@install -m 644 deb/AbiWord_d.1 $(DEB_ROOT)/usr/share/man/man1/.
	@ln -s AbiWord_d.1.gz $(DEB_ROOT)/usr/share/man/man1/AbiWord.1.gz
	@ln -s AbiWord_d.1.gz $(DEB_ROOT)/usr/share/man/man1/abiword.1.gz
	@install -m 644 deb/copyright $(DEB_ROOT)/usr/share/doc/abiword/.
	@install -m 644 deb/changelog $(DEB_ROOT)/usr/share/doc/abiword/changelog.Debian
	@install -m 644 $(OUTDIR)/AbiSuite/readme.txt $(DEB_ROOT)/usr/share/doc/abiword/README
	@cp -R $(OUTDIR)/AbiSuite/AbiWord/sample/* $(DEB_ROOT)/usr/share/doc/abiword/examples/.
	@gzip -9 $(DEB_ROOT)/usr/share/doc/abiword/README
	@gzip -9 $(DEB_ROOT)/usr/share/doc/abiword/changelog.Debian
	@gzip -9 $(DEB_ROOT)/usr/share/man/man1/AbiWord_d.1
#install the share stuff
	@cp -R $(OUTDIR)/AbiSuite/icons/* $(DEB_ROOT)/usr/share/abisuite/icons/.
	@cp -R $(OUTDIR)/AbiSuite/fonts/* $(DEB_ROOT)/usr/share/abisuite/fonts/.
	@cp -R $(OUTDIR)/AbiSuite/AbiWord/strings/* $(DEB_ROOT)/usr/share/abisuite/AbiWord/strings/.
	@cp -R $(OUTDIR)/AbiSuite/AbiWord/help/* $(DEB_ROOT)/usr/share/abisuite/AbiWord/help/.
	@ln -s ../../../lib/ispell/default.hash $(DEB_ROOT)/usr/share/abisuite/dictionary/american.hash
#other debian stuff
	@install -m 644 deb/menu $(DEB_ROOT)/usr/lib/menu/abiword
	@cp deb/control $(DEBIAN_ROOT)/debian/.
	@cp deb/postinst $(DEB_ROOT)/DEBIAN/.
	@cp deb/postrm $(DEB_ROOT)/DEBIAN/.
	@cp deb/prerm $(DEB_ROOT)/DEBIAN/.
	@cp deb/changelog $(DEBIAN_ROOT)/debian/.
	(cd $(DEBIAN_ROOT);dpkg-shlibdeps $(DEB_ROOT)/usr/bin/*_d)
	(cd $(DEBIAN_ROOT);dpkg-gencontrol -v$(ABI_BUILD_VERSION) -isp)
	(fakeroot $(ABI_ROOT)/src/pkg/linux/scripts/install_debian.sh $(DEB_ROOT) $(ABI_ROOT)/dist/abisuite-$(ABI_BUILD_VERSION))
	@mv $(DEBIAN_ROOT)/debian/*.deb $(ABI_ROOT)/dist/

# We're using alien to convert from RPM to DEB package
deb_old: rpm
	@echo "Producing DEB..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))	
	(cd $(DIST); alien --to-deb -c -k abisuite-$(ABI_BUILD_VERSION)-$(ABI_RELEASE_NUMBER).$(OS_ARCH).rpm)

# We're using alien to convert from RPM to SLP package
slp: rpm
	@echo "Producing SLP..."
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))	
	(cd $(DIST); alien --to-slp -c -k abisuite-$(ABI_BUILD_VERSION)-$(ABI_RELEASE_NUMBER).$(OS_ARCH).rpm)

# tgz target is provided by a common Unix makefile
include $(ABI_ROOT)/src/pkg/common/unix/tgz.mk

# tbz target is provided by a common Unix makefile
include $(ABI_ROOT)/src/pkg/common/unix/tbz.mk

# This target should build all packages 
distribution: $(DIST_DEPS)
	echo $(DIST_DEPS)
ifeq ($(WIPE_RPM),1)
	@rm $(DIST)/abisuite-$(ABI_BUILD_VERSION)-$(ABI_RELEASE_NUMBER).$(OS_ARCH).rpm
endif
