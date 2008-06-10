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

ABI_ROOT	:=$(shell pwd)

# This allows one to specify prefix in compile time
ifeq ($(strip $(prefix)),)
prefix          :=/usr/local
endif

##################################################################
## Useful options:
##
##	make	[OUT=<compiler_output_directory>]
##		[DIST=<distribution_output_directory>]
##		[ABI_BUILD_VERSION=x.y.z]
##		[ABI_BUILD_ID=<some_useful_to_you_distinguishing_label>]


default:	compile


test:
	@echo "make test not yet supported with diving Makefile"
	@echo "Please use autoconf build."

##################################################################
## Compile all applications in AbiSuite
## This creates $(OUT)/bin/<programs>

compile:
	@echo Building AbiSuite with [ABI_ROOT=$(ABI_ROOT)]
	$(MAKE) ABI_ROOT=$(ABI_ROOT) -C src

compile_dbg:
	@echo Building AbiSuite with [ABI_ROOT=$(ABI_ROOT)]
	$(MAKE) ABI_ROOT=$(ABI_ROOT) ABI_OPT_DEBUG=1 -C src

##################################################################
## Compile with BiDiRectional support

bidi:
	@echo Building BiDiRectionally enabled AbiSuite with [ABI_ROOT=$(ABI_ROOT)]
	$(MAKE) ABI_ROOT=$(ABI_ROOT) ABI_OPT_BIDI_ENABLED=1 UNIX_CAN_BUILD_DYNAMIC=0 -C src

bidi_dbg:
	@echo Building BiDiRectionally enabled AbiSuite with [ABI_ROOT=$(ABI_ROOT)]
	$(MAKE) ABI_ROOT=$(ABI_ROOT) ABI_OPT_BIDI_ENABLED=1 ABI_OPT_DEBUG=1 UNIX_CAN_BUILD_DYNAMIC=0 -C src

##################################################################

dox:
	@echo Generating documentation
	doxygen

##################################################################

## Quick developer install with no packaging

install:
	@echo Installing AbiSuite with [prefix=$(prefix)]
	$(MAKE) prefix=$(prefix) -C src install

install_dbg:
	@echo Installing AbiSuite with [prefix=$(prefix)]
	$(MAKE) prefix=$(prefix) ABI_OPT_DEBUG=1 -C src install

##################################################################

install_redhat:
	@echo Installing AbiSuite for Red Hat Linux systems
	$(MAKE) -C src install_redhat

##################################################################
## Build system library files (strings, dictionaries, example
## documents, default system profile, etc) in a canonical, 
## non-installed layout.   (This is the layout we'd like to use
## -- and do use for personal builds.  Various systems may want
## a different layout and can use this canonical layout to
## construct the desired layout and installation script.)
## This creates $(OUT)/AbiSuite

canonical:
	@echo Building AbiSuite Canonical Layout
	$(MAKE) ABI_ROOT=$(ABI_ROOT) -C src canonical

##################################################################
## Target to make binary distribution files
## This creates $(DIST)/<platform_specific_installation_packages>

distribution: compile canonical
	$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C src distribution

toolsplugins:
	$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C src toolsplugins

disttools:
	$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C src disttools

impexpplugins:
	$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C src impexpplugins

distimpexp:
	$(MAKE) ABI_ROOT=$(ABI_ROOT) prefix=$(prefix) -C src distimpexp
##################################################################
## Targets to clean up the mess that we make

clean:
	$(MAKE) ABI_ROOT=$(ABI_ROOT) -C src clean

abiclean:
	$(MAKE) ABI_ROOT=$(ABI_ROOT) -C src abiclean

realclean:
	$(MAKE) ABI_ROOT=$(ABI_ROOT) -C src realclean
	rm -rf dist

