#! gmake

## AbiWord
## Copyright (C) 1998-2002 AbiSource, Inc.
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
#REQUIREREALCLEAN=	realclean_LIBICONV_ realclean_LIBWV_ clean_POPT_

ifeq ($(ABI_OPT_CURL),1)
REQUIRE += _PEER_LIBCURL_
endif

TARGET=		build

#    CCSET=CC='cl.exe -nologo' CONFIG_SHELL=sh sh

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
		echo peer zlib not found.  You can check it out of cvs by; \
		echo cvs -d :pserver:anoncvs:anoncvs@anoncvs.abisource.com:/cvsroot checkout zlib; \
		exit 1;						\
	fi

_LIBPNG_:
	@if [ -d $(ABI_ROOT)/../libpng ]; then					\
		echo Found libpng in peer directory;				\
		$(MAKE) -f Makefile.abi ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../libpng;	\
	else									\
		echo peer libpng not found.  You can check it out of cvs by; \
		echo cvs -d :pserver:anoncvs:anoncvs@cvs.abisource.com:/cvsroot checkout libpng; \
		exit 1;						\
	fi

_LIBICONV_:
	@if [ -d $(ABI_ROOT)/../libiconv ]; then			\
		echo Found libiconv in peer directory;			\
		cd $(ABI_ROOT)/../libiconv; 			\
		$(ABICOPY) lib/config.h.msvc lib/config.h;	\
		$(ABICOPY) libcharset/config.h.msvc libcharset/config.h;	\
		$(ABICOPY) libcharset/include/libcharset.h.in libcharset/include/libcharset.h;	\
		$(ABICOPY) include/iconv.h.msvc-static include/iconv.h; \
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../libiconv;	\
	else								\
		echo peer libiconv not found.  You can check it out of cvs by; \
		echo cvs -d :pserver:anoncvs:anoncvs@cvs.abisource.com:/cvsroot checkout libiconv; \
		exit 1;						\
	fi
#			$(CCSET) ./configure --enable-extra-encodings;

_LIBWV_:
	@if [ -d $(ABI_ROOT)/../wv ]; then			\
		echo Found libwv in peer directory;		\
		cd $(ABI_ROOT)/../wv; 			\
		$(ABICOPY) config.h.msvc config.h;	\
		echo PLEASE verify that VERSION in version.c is correct!!!;	\
		$(ABICOPY) version.c.win32 version.c;	\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../wv;  	\
	else							\
		echo peer wv not found.  You can check it out of cvs by; \
		echo cvs -d :pserver:anoncvs:anoncvs@cvs.abisource.com:/cvsroot checkout wv; \
		exit 1;						\
	fi
#			$(CCSET) ./configure --with-peer-iconv=abi;

_POPT_:
	@if [ -d $(ABI_ROOT)/../popt ]; then                       	\
		echo Found popt in peer directory;                      \
		if [ ! -r $(ABI_ROOT)/../popt/config.h ]; then          \
			cd $(ABI_ROOT)/../popt;                         \
			$(ABICOPY) config.h.msvc config.h;	\
		fi;                                                     \
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../popt;                         \
	else										\
		echo peer popt not found.  You can check it out of cvs by; \
		echo cvs -d :pserver:anoncvs:anoncvs@cvs.abisource.com:/cvsroot checkout popt; \
		exit 1;						\
	fi								
#			$(CCSET) ./configure --enable-static --disable-shared;  

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
	@if [ -d $(ABI_ROOT)/../curl/lib ]; then	\
		echo Found libcurl in peer directory;	\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../curl/lib;		\
	else									\
		echo libcurl not found.  You can check it out of cvs by; \
		echo cvs -d :pserver:anoncvs:anoncvs@cvs.abisource.com:/cvsroot checkout curl; \
		echo or download from http://curl.haxx.se and extract the library as a peer;	\
		echo directory named curl;			\
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

