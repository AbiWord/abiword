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

ABI_ROOT:=$(shell pwd)/../../../..

include $(ABI_ROOT)/src/config/abi_defs.mk

ifdef ABI_OPT_LIBXML2
REQUIRE=
else
REQUIRE=	_EXPAT_ 
endif
REQUIRE+=	_PSICONV_
TARGET=		build

include $(ABI_ROOT)/src/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

#################################################################

_EXPAT_:
	@if [ -d $(ABI_ROOT)/../expat ]; then						\
		echo "Found expat in peer directory";					\
		if [ ! -r $(ABI_ROOT)/../expat/config.hin ]; then \
			cd $(ABI_ROOT)/../expat; \
			./configure --disable-shared --enable-static; \
		fi; \
		$(MAKE) ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../expat;			\
	elif [ -e /usr/include/expat.h -o -e /usr/local/include/expat.h ]; then \
		echo "*** Found a shared library version of expat.  I'm going to use "; \
		echo "*** that.  If it doesn't work, you should use the version "; \
		echo "*** distributed with AbiWord."; \
		echo ; \
	else										\
		echo "ERROR -- expat not found";						\
		echo "You need to get expat from the AbiSource downloads site";		\
		echo "It is identical to expat from expat.sourceforge.net, except for";		\
		echo "minor modifications to integrate with the AbiSource build system.";	\
		exit 1;									\
	fi

_PSICONV_:
	@if [ -r $(ABI_ROOT)/../psiconv ]; then \
		echo "Found psiconv in peer directory"; \
		if [ ! -r $(ABI_ROOT)/../psiconv/config.h ]; then \
			cd $(ABI_ROOT)/../psiconv; \
			./configure --disable-shared --enable-static; \
		fi; \
		$(MAKE) -f Makefile -C $(ABI_ROOT)/../psiconv; \
	elif [ -d /usr/include/psiconv -o -d /usr/local/include/psiconv ]; then \
		echo "*** Found an install of psiconv. I'm going to use it; if it is too old,"; \
		echo "*** you will have to upgrade it or get the version distributed with AbiWord."; \
		echo "*** You can find psiconv at http://huizen.dds.nl/~frodol/psiconv/"; \
		echo ; \
	else \
		echo "Psiconv not found. Get the version distributed with AbiWord and unpack it" ;\
		echo "in a peer directory; or get it from http://huizen.dds.nl/~frodol/psiconv and" ;\
		echo "install it separately." ;\
		exit 1;\
	fi
