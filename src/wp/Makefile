#! gmake

## AbiWord
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

ABI_ROOT:=$(shell pwd)/../..
include $(ABI_ROOT)/src/config/abi_defs_wp.mk

## Note: main must be last

DIRS=		ap impexp main

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## Build system library files (strings, dictionaries, example
## documents, default system profile, etc) in a canonical, 
## non-installed layout.

## CANON_AppDir is where we put application-specific stuff in the
## canonical layout.
CANON_AppDir=	$(CANONDIR)/$(ABI_APPLIBDIR)

## CANON_Lang is a wildcard to match language strings like "en-US"
## and "no-BOK" (and not "CVS").
CANON_Lang=	[a-z][a-z]?[A-Z][A-Z]*

canonical:
	@$(subst xxxx,$(CANON_AppDir),$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(CANON_AppDir)/strings,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(CANON_AppDir)/help,$(VERIFY_DIRECTORY))
	@$(subst xxxx,$(CANON_AppDir)/sample,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(USERDIR)/wp/readme.txt $(CANON_AppDir)
	@$(ABICOPY) $(USERDIR)/wp/system.profile $(CANON_AppDir)
	@$(ABICOPY) $(USERDIR)/wp/strings/*.strings $(CANON_AppDir)/strings
	(cd $(USERDIR)/wp/help; tar cf - images ./*.* $(CANON_Lang)/*.* $(CANON_Lang)/*/*.*) | (cd $(CANON_AppDir)/help; tar xf -)
	(cd $(USERDIR)/wp/sample; tar cf - $(CANON_Lang)/*.*) | (cd $(CANON_AppDir)/sample; tar xf -)

