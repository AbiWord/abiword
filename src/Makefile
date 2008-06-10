#! gmake

## AbiSource Applications
## Copyright (C) 1998 AbiSource, Inc.
## Copyright (C) 2008 AbiSource Corporation B.V.
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

DIRS=		config tools other af text wp 
REQUIRECLEAN=
REQUIREREALCLEAN=		clean_ABI_ clean_ZLIB_ clean_LIBPNG_ clean_LIBWV_ clean_LIBICONV_ clean_POPT_

ifeq ($(ABI_OPT_PEER_EXPAT),1)
REQUIRECLEAN+= clean_EXPAT_
REQUIREREALCLEAN+= realclean_EXPAT_
endif


include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## Targets for cleaning up

abiclean::
	rm -f $(OBJDIR)/*.*

clean:: $(REQUIRECLEAN)
	rm -rf $(OBJDIR)

realclean:: $(REQUIREREALCLEAN)

##################################################################
## Build system library files (strings, dictionaries, example
## documents, default system profile, etc) in a canonical, 
## non-installed layout.

## Set the following to a list of pathnames to be copied to $(CANONDIR)
CANON_TOPLEVELFILES=	$(ABI_ROOT)/COPYING $(ABI_ROOT)/user/wp/readme.txt $(ABI_ROOT)/user/wp/readme.abw

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

# each installer requires abidistfiles for necessary but fairly constant files, 
# so we check for those here (TODO : move check somewhere else?)
	@if [ -d $(ABI_ROOT)/../abidistfiles ]; then		\
		echo Found abidistfiles in peer directory;		\
	else							\
		echo ;						\
		echo " - abidistfiles not found";			\
		echo "  'make canonical', 'make install', and 'make distribution' require ";	\
		echo "  the abidistfiles/ package be available as a peer to the abi/ tree.";	\
		echo ;						\
		exit 1;						\
	fi

# copy the various dictionary hash files, only used in non-pspell builds
ifeq ($(ABI_OPT_PSPELL),1)
	echo "Using Pspell build, not installing ispell hash files"
else
	# We currently only ship american.hash
	@$(subst xxxx,$(CANONDIR)/dictionary,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(ABI_ROOT)/../abidistfiles/dictionary/$(OS_ENDIAN).american.hash $(CANONDIR)/dictionary/american.hash
	@$(ABICOPY) $(ABI_ROOT)/../abidistfiles/dictionary/*.xml $(CANONDIR)/dictionary	
endif

ifeq ($(ABI_NATIVE),unix)
# icons for X window manager use
	@$(subst xxxx,$(CANONDIR)/icons,$(VERIFY_DIRECTORY))
	(cd $(ABI_ROOT)/../abidistfiles/icons; cp abi* $(CANONDIR)/icons)
endif

	@$(subst xxxx,$(CANONDIR)/clipart,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(ABI_ROOT)/../abidistfiles/clipart/*.png $(CANONDIR)/clipart

	@$(subst xxxx,$(CANONDIR)/templates,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(ABI_ROOT)/../abidistfiles/templates/*.awt* $(CANONDIR)/templates

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
    disttools: distribution
    distimpexp: distribution

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

    toolsplugins:
	@if [ -d $(ABI_ROOT)/../abiword-plugins ]; then \
		echo Found abiword-plugins in peer directory; \
	else \
		echo ; \
		echo " - abiword_plugins not found"; \
		echo "  'make disttools' requires the abiword-plugins/ package be available as"; \
		echo "  a peer to the abi/ tree."; \
		echo "  You can check it out of cvs by running the following command from the"; \
		echo "  same directory you checked out abi into: "; \
		echo "  cvs -d :pserver:anoncvs:anoncvs@cvs.abisource.com:/cvsroot co abiword-plugins"; \
		exit 1; \
	fi
	@echo Building tools plugins...
	@$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C $(ABI_ROOT)/../abiword-plugins/tools all
	@$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C $(ABI_ROOT)/../abiword-plugins/wp/scripts/shell
	
    disttools: toolsplugins
	@echo Building distribution tools plugin package for $(ABIPKGDIR)...
	@$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C pkg/$(ABIPKGDIR) disttools

    impexpplugins:
	@if [ -d $(ABI_ROOT)/../abiword-plugins ]; then \
		echo Found abiword-plugins in peer directory; \
	else \
		echo ; \
		echo " - abiword_plugins not found"; \
		echo "  'make distimpexp' requires the abiword-plugins/ package be available as"; \
		echo "  a peer to the abi/ tree."; \
		echo "  You can check it out of cvs by running the following command from the"; \
		echo "  same directory you checked out abi into: "; \
		echo "  cvs -d :pserver:anoncvs:anoncvs@cvs.abisource.com:/cvsroot co abiword-plugins"; \
		exit 1; \
	fi
	@echo Building importer/exporter plugins...
	@$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C $(ABI_ROOT)/../abiword-plugins/wp/impexp all
	
    distimpexp: impexpplugins
	@echo Building distribution importer/exporter plugin package for $(ABIPKGDIR)...
	@$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C pkg/$(ABIPKGDIR) distimpexp

  endif
else
  disttools: distribution
  distimpexp: distribution
  distribution:
	@echo
	@echo Error -- No package script defined for this platform.
	@echo Error -- Please make sure ABIPKGDIR is set in
	@echo Error -- the proper 'abi/src/config/platforms/*.mk'
	@echo Error -- makefile.
	@echo
	@exit 1
endif

##################################################################
## Targets for cleaning peer libraries. 
##
## Note that all peer libraries need an opportunity to realclean, 
## but any peer library using Makefile.abi doesn't need to clean
## since all outputs were in our OBJDIR tree already.
##
## Thus, for Makefile.abi peers, you'll only see one rule here
## (which gets included in REQUIREREALCLEAN above).  Peers using
## their own autoconf magic have two rules.    

clean_ZLIB_:
	@if [ -d $(ABI_ROOT)/../zlib ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../zlib clean;	\
	fi

clean_LIBPNG_:
	@if [ -d $(ABI_ROOT)/../libpng ]; then		\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../libpng clean;	\
	fi

clean_PSICONV_:
	@if [ -d $(ABI_ROOT)/../psiconv ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../psiconv clean;	\
	fi

clean_EXPAT_:
ifeq ($(ABI_FE), Win32)
	@if [ -d $(ABI_ROOT)/../expat ]; then		\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../expat clean;	\
	fi
else
	@if [ -d $(ABI_ROOT)/../expat ]; then		\
		$(MAKE) -f Makefile -C $(ABI_ROOT)/../expat clean;	\
	fi
endif

# we have to make a realclean here so the configure script reruns
# if no makefile exists, it is already clean
clean_LIBWV_:
ifneq ($(OS_NAME),WIN32)
	@if [ -d $(ABI_ROOT)/../wv -a -f $(ABI_ROOT)/../wv/Makefile ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../wv realclean;	\
	fi	
else
	$(MAKE) -fMakefile.abi -C $(ABI_ROOT)/../wv realclean;
endif

# we have to make a distclean here to ensure that the configure script reruns
# if no makefile exists, it is already clean
clean_LIBICONV_:
ifneq ($(OS_NAME),WIN32)
	@if [ -d $(ABI_ROOT)/../libiconv -a -f $(ABI_ROOT)/../libiconv/Makefile ]; then	\
		$(MAKE) -C $(ABI_ROOT)/../libiconv distclean;	\
	fi	
else
	$(MAKE) -fMakefile.abi -C $(ABI_ROOT)/../libiconv clean;
	$(RM) $(ABI_ROOT)/../libiconv/lib/config.h;
	$(RM) $(ABI_ROOT)/../libiconv/libcharset/config.h;
	$(RM) $(ABI_ROOT)/../libiconv/include/iconv.h;
endif

clean_POPT_:
ifeq ($(OS_NAME),WIN32)
	@if [ -d $(ABI_ROOT)/../popt ]; then              \
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../popt clean;     \
		$(RM)	$(ABI_ROOT)/../popt/config.h;	\
	fi
endif

realclean_PSICONV_:
	@if [ -d $(ABI_ROOT)/../psiconv -a -f $(ABI_ROOT)/../psiconv/psiconv/Makefile ]; then	\
		$(MAKE) -C $(ABI_ROOT)/../psiconv distclean;	\
	fi	

realclean_EXPAT_:
ifneq ($(ABI_FE),Win32)
	@if [ -d $(ABI_ROOT)/../expat -a -f $(ABI_ROOT)/../expat/lib/Makefile ]; then	\
		$(MAKE) -C $(ABI_ROOT)/../expat distclean;	\
	fi	
else
	@if [ -d $(ABI_ROOT)/../expat ]; then		\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../expat clean;	\
		$(RM) $(ABI_ROOT)/../expat/config.h;	\
		$(RM) $(ABI_ROOT)/../expat/lib/config.h;	\
	fi
endif

clean_ABI_:
	rm -rf $(OUTDIR) $(DIST)

