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

REQUIRE=	_ZLIB_ _LIBPNG_ _LIBICONV_ _LIBWV_ _POPT_ _NSIS_ _FRIBIDI_
ifeq ($(OS_NAME),MINGW32)
    CCSET=
else
    CCSET=CC='cl.exe -nologo' CONFIG_SHELL=bash.exe bash
endif
REQUIREREALCLEAN=	realclean_LIBICONV_ realclean_LIBWV_
ifeq ($(ABI_OPT_CURL),1)
REQUIRE += _PEER_LIBCURL_
endif
TARGET=		build

include $(ABI_ROOT)/src/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

realclean::		$(REQUIREREALCLEAN)


#################################################################

_ZLIB_:
	@if [ -d $(ABI_ROOT)/../zlib ]; then					\
		echo Found zlib in peer directory;				\
		$(MAKE) ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../zlib;		\
	else									\
		echo zlib not found;						\
		echo TODO provide a nicer error message;			\
		echo TODO with instructions for obtaining it;			\
		exit 1;								\
	fi

_LIBPNG_:
	@if [ -d $(ABI_ROOT)/../libpng ]; then					\
		echo Found libpng in peer directory;				\
		$(MAKE) -f Makefile.abi ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../libpng;	\
	else									\
		echo libpng not found;						\
		echo TODO provide a nicer error message;			\
		echo TODO with instructions for obtaining it;			\
		exit 1;								\
	fi

_LIBICONV_:
	@if [ -d $(ABI_ROOT)/../libiconv ]; then			\
		echo Found libiconv in peer directory;			\
		if [ ! -r $(ABI_ROOT)/../libiconv/lib/config.h ]; then	\
			cd $(ABI_ROOT)/../libiconv; 			\
			$(CCSET) ./configure --enable-extra-encodings; \
		fi;						\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../libiconv;	\
	else								\
		echo libiconv not found;				\
		echo TODO provide a nicer error message;		\
		echo TODO with instructions for obtaining it;		\
		exit 1;							\
	fi

_LIBWV_:
	@if [ -d $(ABI_ROOT)/../wv ]; then			\
		echo Found libwv in peer directory;		\
		if [ ! -r $(ABI_ROOT)/../wv/config.h ]; then	\
			cd $(ABI_ROOT)/../wv; 			\
			$(CCSET) ./configure --with-peer-iconv=abi;	\
		fi;						\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../wv;  	\
	else							\
		echo wv not found;				\
		echo TODO provide a nicer error message;	\
		echo TODO with instructions for obtaining it;	\
		exit 1;						\
	fi


_POPT_:
	@if [ -d $(ABI_ROOT)/../popt ]; then                       	\
		echo Found popt in peer directory;                      \
		if [ ! -r $(ABI_ROOT)/../popt/config.h ]; then          \
			cd $(ABI_ROOT)/../popt;                         \
			$(CCSET) ./configure --enable-static --disable-shared;   \
		fi;                                                     \
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../popt;                         \
	else										\
		echo peer popt not found.  You can check it out of cvs by; \
		cvs -d :pserver:anoncvs:anoncvs@cvs.abisource.com:/cvsroot checkout popt; \
		exit 1;						\
	fi								


_NSIS_:
	@if [ -d $(ABI_ROOT)/../nsis ]; then			\
		echo Found nsis in peer directory;		\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../nsis;		\
	else									\
		echo nsis not found;						\
		echo TODO provide a nicer error message;			\
		echo TODO with instructions for obtaining it;			\
		exit 1;								\
	fi

_PEER_LIBCURL_:
	@if [ -d $(ABI_ROOT)/../libcurl/lib ]; then	\
		echo Found libcurl in peer directory;	\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../libcurl/lib;		\
	else									\
		echo libcurl not found;						\
		echo Please extract the curl library as a peer;			\
		echo directory named libcurl;			\
		exit 1;								\
	fi
#		cd $(ABI_ROOT)/../libcurl/lib
#		nmake /f Makefile.vc6 cfg=release

# TODO: update to support building statically or as DLL
_FRIBIDI_:
	@if [ -d $(ABI_ROOT)/../fribidi ]; then					\
		echo Found fribidi in peer directory;				\
		$(MAKE) -f Makefile.abi ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../fribidi;	\
	else									\
		echo fribidi not found;						\
		echo Please checkout the fribidi module from cvs or \
		echo download it from where you obtained your AbiWord source. \
		exit 1;								\
	fi

#################################################################

realclean_LIBICONV_:
	@if [ -d $(ABI_ROOT)/../libiconv ]; then			\
		$(MAKE) -C $(ABI_ROOT)/../libiconv distclean;	\
	fi

realclean_LIBWV_:
	@if [ -d $(ABI_ROOT)/../wv ]; then			\
		$(MAKE) -C $(ABI_ROOT)/../wv realclean;	\
	fi

clean_POPT_:
	@if [ -d $(ABI_ROOT)/../popt ]; then              \
		$(MAKE) -C $(ABI_ROOT)/../popt clean;     \
	fi
