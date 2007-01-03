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

# If peer iconv is used, it must be built before wv, or at least
# iconv.h must be generated (or copied from template) before building wv


REQUIRE=	_ZLIB_ _LIBPNG_ 
ifeq ($(ABI_REQUIRE_PEER_ICONV),1)
REQUIRE+=	_LIBICONV_
endif
ifeq ($(HAVE_WV_PEER),1)
REQUIRE+=	_LIBWV_
endif
REQUIRE+=	_POPT_ _FRIBIDI_
#REQUIREREALCLEAN=	realclean_LIBICONV_ realclean_LIBWV_ clean_POPT_

TARGET=		build

#    CCSET=CC='cl.exe -nologo' CONFIG_SHELL=sh sh

include $(ABI_ROOT)/src/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

realclean::		$(REQUIREREALCLEAN)


#################################################################

_ZLIB_:
	@if [ -d $(ABI_ROOT)/../libs/zlib ]; then	\
		echo Using precompiled zlib in libs directory;				\
		if [ $(OS_NAME) = MINGW32 ]; then	\
			dlltool -D zlib1.dll -d $(ABI_ROOT)/../libs/zlib/lib/zlib.def	\
		-l $(ABI_ROOT)/../libs/zlib/lib/libzdll.a;	\
		fi	\
	else	if [ -d $(ABI_ROOT)/../zlib ]; then					\
			echo "Found zlib in peer directory";				\
			echo "Building zlib DLL, NOT for redistribution!" ;	\
			echo "For redistribution, Please obtain official win32 zlib DLL";	\
			echo "from http://www.gzip.org/zlib/ and unzip into"; \
			echo "$(ABI_ROOT)/../libs/zlib/ ";	\
			$(MAKE) ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../zlib -fmakefile.abi;		\
		else									\
			echo peer zlib not found.  You can check it out of cvs by; \
			echo cvs -d :pserver:anoncvs:anoncvs@anoncvs.abisource.com:/cvsroot checkout zlib; \
			echo or preferably, download the precompiled zlib DLL; \
			echo from http://www.gzip.org/zlib/ and unzip into; \
			echo $(ABI_ROOT)/../libs/zlib/ ;	\
			exit 1;						\
		fi	\
	fi

_LIBPNG_:
	@if [ -d $(ABI_ROOT)/../libpng ]; then					\
		echo Found libpng in peer directory;				\
		$(MAKE) -f Makefile.abi.head ABI_ROOT=$(ABI_ROOT) -C $(ABI_ROOT)/../libpng;	\
	else									\
		echo peer libpng not found.  You can check it out of cvs by; \
		echo cvs -d :pserver:anoncvs:anoncvs@cvs.abisource.com:/cvsroot checkout libpng; \
		exit 1;						\
	fi

_LIBICONV_:
	@if [ -d $(ABI_ROOT)/../libiconv ]; then			\
		echo Found libiconv in peer directory;			\
		cd $(ABI_ROOT)/../libiconv;  \
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
		if [ ! -r config.h ]; then	\
			$(ABICOPY) config.h.msvc config.h;	\
		fi;	\
		if [ ! -r version.c ]; then 	\
			echo PLEASE verify that VERSION in version.c is correct!!!;	\
			$(ABICOPY) version.c.win32 version.c;	\
		fi;	\
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

