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

ABI_ROOT:=$(shell pwd)/..
ABI_DEPTH=.

include $(ABI_DEPTH)/config/abi_defs.mk

DIRS=		config other af text wp ask

include $(ABI_DEPTH)/config/abi_rules.mk

##################################################################
## Targets for cleaning up

clean::
	rm -rf $(OBJDIR)

realclean::
	rm -rf $(OUTDIR) $(DIST)

##################################################################
## Build system library files (strings, dictionaries, example
## documents, default system profile, etc) in a canonical, 
## non-installed layout.

## Set the following to a list of pathnames to be copied to $(CANONDIR)
CANON_TOPLEVELFILES=	$(ABI_DEPTH)/../COPYING

## Set the following to a list of directories (probably a subset of DIRS)
## that have work to do.
CANON_SubDirs=		wp

canonical:
	@$(subst xxxx,$(CANONDIR),$(VERIFY_DIRECTORY))
	@for k in $(CANON_TOPLEVELFILES); do			\
		$(ABICOPY) $$k $(CANONDIR);			\
	done
	@for k in $(CANON_SubDirs); do				\
		$(MAKE) ABI_ROOT=$(ABI_ROOT) -C $$k canonical;	\
	done
	(cd $(ABI_ROOT)/../abidistfiles; tar -cf - dictionary/*.hash) | (cd $(CANONDIR); tar -xf -)
ifeq ($(ABI_NATIVE),unix)
	@$(subst xxxx,$(CANONDIR)/fonts,$(VERIFY_DIRECTORY))
	$(ABICOPY) $(ABI_ROOT)/../unixfonts/fonts.dir $(CANONDIR)/fonts
	$(ABICOPY) $(ABI_ROOT)/../unixfonts/fonts/*.pfa $(CANONDIR)/fonts
	$(ABICOPY) $(ABI_ROOT)/../unixfonts/fonts/*.afm $(CANONDIR)/fonts
endif

##################################################################
## Target for building a distribution package from the compiled
## sources.  Since each platform has it's own quirks, the config
## files in abi/src/config/platforms/*.mk must specify which
## package script to run.  Note, some platform scripts may build
## more than one package (like Linux with .tgz, .deb, .rpm)
##
## I'm not going to set the dependency to the executable here
## because this probably needs to be a separate step anyway and
## it is already set at the /abi level.

ifneq ($(ABIPKGDIR),)

distribution:
	@echo Building distribution packages for $(ABIPKGDIR)...
	@$(MAKE) ABI_ROOT=$(ABI_ROOT) -C pkg/$(ABIPKGDIR) distribution

else

distribution:
	@echo Error -- No package script defined for this platform.
	@echo Error -- Please make sure ABIPKGDIR is set in
	@echo Error -- the proper 'abi/src/config/platforms/*.mk'
	@echo Error -- makefile.
	@exit 1
endif
