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

ABI_DEPTH=../..
include $(ABI_DEPTH)/config/abi_defs.mk

REQUIRE=		_ZLIB_ _LIBPNG_
REQUIRECLEAN=		clean_ZLIB_ clean_LIBPNG_
REQUIREREALCLEAN=	$(REQUIRECLEAN)

TARGET=		build

include $(ABI_DEPTH)/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

clean::		$(REQUIRECLEAN)

realclean::	$(REQUIREREALCLEAN)

#################################################################

_ZLIB_:
	@if [ -d ../../$(ABI_DEPTH)/zlib ]; then		\
		echo Found zlib in peer directory;		\
		$(MAKE) -C ../../$(ABI_DEPTH)/zlib;		\
	else							\
		echo zlib not found;				\
		echo TODO provide a nicer error message;	\
		echo TODO with instructions for obtaining it;	\
		exit 1;						\
	fi

_LIBPNG_:
	@if [ -d ../../$(ABI_DEPTH)/libpng ]; then		\
		echo Found libpng in peer directory;		\
		$(MAKE) -C ../../$(ABI_DEPTH)/libpng;		\
	else							\
		echo libpng not found;				\
		echo TODO provide a nicer error message;	\
		echo TODO with instructions for obtaining it;	\
		exit 1;						\
	fi


#################################################################

clean_ZLIB_:
	@if [ -d ../../$(ABI_DEPTH)/zlib ]; then		\
		$(MAKE) -C ../../$(ABI_DEPTH)/zlib clean;	\
	fi

clean_LIBPNG_:
	@if [ -d ../../$(ABI_DEPTH)/libpng ]; then		\
		$(MAKE) -C ../../$(ABI_DEPTH)/libpng clean;	\
	fi
