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

REQUIRE=		_ZLIB_ _LIBPNG_ _LIBICONV_ _LIBWV_ _ABIDISTFILES_ 
REQUIRECLEAN=		clean_ZLIB_ clean_LIBPNG_ clean_LIBICONV clean_LIBWV_
REQUIREREALCLEAN=	$(REQUIRECLEAN) 

ifeq ($(ABI_OPT_PEER_EXPAT),1)
	XML_OPTION=--with-expat=peer
else
	XML_OPTION=--with-libxml2
endif

TARGET=		build

include $(ABI_ROOT)/src/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

clean::		$(REQUIRECLEAN)

realclean::	$(REQUIREREALCLEAN)

#################################################################

_ZLIB_:
	@if [ -r /usr/include/zlib.h ]; then					\
		echo "*** Found zlib in system locations.";			\
	else									\
		echo zlib not found;						\
		exit 1;								\
	fi

_LIBPNG_:
	@if [ -r /usr/include/png.h ]; then					\
		echo "Found libpng in system locations.";				\
	else									\
		echo libpng not found;						\
		echo libpng is availible from the QNX CD with third party software;\
		echo It is also availible from the qnx.wox.org/repository/6.2/ repository.;\
		exit 1;								\
	fi

# Right now, we can only build wv as a peer, not in the
# system directories.

_LIBWV_:
	@if [ -r $(ABI_ROOT)/../wv ]; then			\
		echo Found libwv in peer directory;		\
		if [ ! -r $(ABI_ROOT)/../wv/config.h ]; then	\
			cd $(ABI_ROOT)/../wv; 			\
			./configure --with-peer-iconv=abi $(XML_OPTION);	\
		fi;						\
		$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../wv;  	\
	else							\
		echo wv not found;				\
		echo wv must be installed as a peer to the abi/ directory;	\
		echo Instructions for obtaining it can be found at;		\
		echo http://www.abisource.com/dev_download.phtml		\
		exit 1;						\
	fi

# if this target fails things aren't fatal, the user
# just gets a warning
_ABIDISTFILES_:
	@if [ -d $(ABI_ROOT)/../abidistfiles ]; then		\
		echo Found abidistfiles in peer directory;		\
	else							\
		echo ;						\
		echo " - abidistfiles not found";			\
		echo "*** You will not be able to use the 'make install'";	\
		echo "*** target without abidistfiles available as a peer";		\
		echo "*** to the abi/ tree.";			\
		echo ;						\
	fi

# On machines that do _NOT_ have libiconv in the C library, we
# force a build as a peer.  We don't detect whether you have libiconv
# (specifically iconv.h) as part of your C library, or whether it 
# was something you installed yourself.  

_LIBICONV_:
	@if [ ! -r /usr/include/iconv.h -a ! -r /usr/local/include/iconv.h ]; then	\
		if [ -d $(ABI_ROOT)/../libiconv ]; then                        	\
			echo Found libiconv in peer directory;                  \
			if [ ! -r $(ABI_ROOT)/../libiconv/lib/config.h ]; then  \
				cd $(ABI_ROOT)/../libiconv;                     \
				./configure --enable-extra-encodings;                                    \
			fi;                                                     \
			$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../libiconv;     \
		else                                                            \
			echo libiconv not found;                                \
			echo TODO provide a nicer error message;                \
			echo TODO with instructions for obtaining it;           \
			exit 1;                                                 \
		fi								\
	else									\
		echo ;								\
		echo "*** Found iconv functions in system locations.  I'm going to trust that these";	\
		echo "*** are the versions of the libraries we should link against.";			\
		echo ;								\
	fi


#################################################################

clean_LIBWV_:
	@if [ -d $(ABI_ROOT)/../wv ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../wv clean;	\

clean_LIBICONV_:
	@if [ -d $(ABI_ROOT)/../libiconv ]; then              \
		$(MAKE) -C $(ABI_ROOT)/../libiconv clean;     \
