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
include $(ABI_DEPTH)/config/abi_rules.mk

##################################################################
## makefile target to make binary distribution for this platform.

distribution:	tgz

##################################################################
## tgz -- this is a very simple tree of the essentials.  this is
##		just a tar of everything and can be put in /usr/local
##		and with a few symbolic links should work just fine....

tgz_dirs=	bin AbiSuite
tgz:
	@echo Building .tar.gz package...
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(DIST)/$(PKGBASENAME),$(VERIFY_DIRECTORY))
	(cd $(OUTDIR); tar -cf - $(tgz_dirs)) | (cd $(DIST)/$(PKGBASENAME); tar -xf -)
	(cd $(DIST)/$(PKGBASENAME)/bin; strip *)
	(cd $(DIST); tar -czf $(PKGBASENAME).tar.gz $(PKGBASENAME))
	(cd $(DIST); rm -rf $(PKGBASENAME))

