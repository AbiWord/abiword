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

ABI_DEPTH=../../..

include $(ABI_DEPTH)/config/abi_defs.mk

ABI_ROOT := $(shell pwd)/../../../..

REQUIRE=	_EXPAT_ 
TARGET=		build

include $(ABI_DEPTH)/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

#################################################################

_EXPAT_:
	@if [ -d $(ABI_ROOT)/../expat ]; then						\
		echo Found expat in peer directory;					\
		$(MAKE) ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../expat;			\
	else										\
		echo ERROR -- expat not found;						\
		echo You need to get expat from the AbiSource downloads site;		\
		echo It is identical to expat from www.jclark.com, except for;		\
		echo minor modifications to integrate with the AbiSource build system.;	\
		exit 1;									\
	fi
