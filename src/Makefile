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

include $(ABI_ROOT)/src/config/abi_defs.mk

DIRS=		config tools other af text wp ask
REQUIREREALCLEAN=		clean_ABI_ clean_ZLIB_ clean_LIBPNG_ clean_LIBWV_ clean_LIBICONV_ clean_PSICONV_


include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## Targets for cleaning up

abiclean::
	rm -f $(OBJDIR)/*.*

clean::
	rm -rf $(OBJDIR)

realclean:: $(REQUIREREALCLEAN)

##################################################################
## Build system library files (strings, dictionaries, example
## documents, default system profile, etc) in a canonical, 
## non-installed layout.

## Set the following to a list of pathnames to be copied to $(CANONDIR)
CANON_TOPLEVELFILES=	$(ABI_ROOT)/COPYING $(ABI_ROOT)/user/wp/readme.txt

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
# copy the various dictionary hash files
	@$(subst xxxx,$(CANONDIR)/dictionary,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(ABI_ROOT)/../abidistfiles/dictionary/$(OS_ENDIAN).*.hash $(CANONDIR)/dictionary
	@(cd $(CANONDIR)/dictionary; 				\
		for k in *.hash; do				\
			cp $$k american.hash;		\
			rm $$k ;			\
		done)
#	(cd $(ABI_ROOT)/../abidistfiles; tar cf - dictionary/*.hash) | (cd $(CANONDIR); tar xf -)
ifeq ($(ABI_NATIVE),unix)
# each Unix installer requires both unixfonts and abidistfiles, 
# so we check for those here (TODO : move them somewhere else?)
# if this target fails things aren't fatal, the user
# just gets a warning (TODO: add a message to the actual
# "make install" target to alert)
	@if [ -d $(ABI_ROOT)/../unixfonts ]; then		\
		echo Found unixfonts in peer directory;		\
	else							\
		echo ;						\
		echo " - unixfonts not found";			\
		echo "  'make canonical' and 'make install' require the unixfonts/ package be";	\
		echo "  available as a peer to the abi/ tree.";			\
		echo ;						\
		exit 1;						\
	fi
	@if [ -d $(ABI_ROOT)/../abidistfiles ]; then		\
		echo Found abidistfiles in peer directory;		\
	else							\
		echo ;						\
		echo " - abidistfiles not found";			\
		echo "  'make canonical' and 'make install' require the abidistfiles/ package be";	\
		echo "  available as a peer to the abi/ tree.";			\
		echo ;						\
		exit 1;						\
	fi
# icons for X window manager use
	@$(subst xxxx,$(CANONDIR)/icons,$(VERIFY_DIRECTORY))
	(cd $(ABI_ROOT)/../abidistfiles/icons; cp *_48* $(CANONDIR)/icons)
# font stuff for installation
	@$(subst xxxx,$(CANONDIR)/fonts,$(VERIFY_DIRECTORY))
	(cd $(ABI_ROOT)/../unixfonts/fonts ; cp *.* $(CANONDIR)/fonts)
endif

##################################################################
## Targets for installs.  There is a generic install method, for
## developers mostly (make install), which just puts things in
## $prefix/AbiSuite and $prefix/bin.  /usr/local is the default
## $prefix value.
##
## Also here are make_<dist> targets for supported distribution
## installations.  Each of these targets passes the appropriate
## targets down to the packaging levels.  These targets are for
## use with automated packaging systems only; real people should
## never need to use these unless they have a good reason.

install_redhat: build canonical
ifeq ($(ABI_NATIVE),unix)
	$(MAKE) -C pkg/$(ABIPKGDIR) install_redhat
else	
	@echo
	@echo Only Unix people do \'make install_redhat\', and even then
	@echo only very rarely.
	@echo
endif

# The generic target
install: build canonical
ifeq ($(ABI_NATIVE),unix)
	@echo
	@echo Using prefix value of [$(prefix)]...
	@echo
	@$(MAKE) prefix=$(prefix) ABI_ROOT=$(ABI_ROOT) -C pkg/$(ABIPKGDIR) install
else
	@echo
	@echo Only Unix people do \'make install\'.
	@echo
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

ifeq ($(ABI_BUILD_VERSION),unnumbered)
distribution:
	@echo 
	@echo Error -- ABI_BUILD_VERSION is not set for this build.
	@echo Error -- Set this variable before building packages
	@echo Error -- for distribution.
	@echo
	@exit 1
else
distribution:
	@echo Building distribution packages for $(ABIPKGDIR)...
	@$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C pkg/$(ABIPKGDIR) distribution
endif

else

distribution:
	@echo
	@echo Error -- No package script defined for this platform.
	@echo Error -- Please make sure ABIPKGDIR is set in
	@echo Error -- the proper 'abi/src/config/platforms/*.mk'
	@echo Error -- makefile.
	@echo
	@exit 1
endif

#################################################################

clean_ZLIB_:
	@if [ -d $(ABI_ROOT)/../zlib ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../zlib clean;	\
	fi

clean_LIBPNG_:
	@if [ -d $(ABI_ROOT)/../libpng ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../libpng clean;	\
	fi

# we have to make a realclean here so the configure script reruns
# if no makefile exists, it is already clean
clean_LIBWV_:
	@if [ -d $(ABI_ROOT)/../wv -a -f $(ABI_ROOT)/../wv/Makefile ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../wv realclean;	\
	fi	

# we have to make a distclean here to ensure that the configure script reruns
# if no makefile exists, it is already clean
clean_LIBICONV_:
	@if [ -d $(ABI_ROOT)/../libiconv -a -f $(ABI_ROOT)/../libiconv/Makefile ]; then	\
		$(MAKE) -C $(ABI_ROOT)/../libiconv distclean;	\
	fi	

clean_PSICONV_:
	@if [ -d $(ABI_ROOT)/../psiconv -a -f $(ABI_ROOT)/../psiconv/psiconv/Makefile ]; then	\
		$(MAKE) -C $(ABI_ROOT)/../psiconv distclean;	\
	fi	

clean_EXPAT_:
	@if [ -d $(ABI_ROOT)/../expat -a -f $(ABI_ROOT)/../expat/lib/Makefile ]; then	\
		$(MAKE) -C $(ABI_ROOT)/../expat distclean;	\
	fi	

clean_ABI_:
	rm -rf $(OUTDIR) $(DIST)

