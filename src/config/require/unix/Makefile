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

REQUIRE=		_GCC_ _GLIB_ _GTK_ _ZLIB_ _LIBPNG_ _FONTS_ #_COLE_
REQUIRECLEAN=		clean_ZLIB_ clean_LIBPNG_ #clean_COLE_
REQUIREREALCLEAN=	$(REQUIRECLEAN) clean_FONTS_

TARGET=		build

include $(ABI_DEPTH)/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

clean::		$(REQUIRECLEAN)

realclean::	$(REQUIREREALCLEAN)

#################################################################

_GCC_:
	@echo TODO do we need to test for GCC or EGCS and a version number?

REQ_GTK_MINOR=1
REQ_GTK_BUILD=13

_GLIB_:
	@glib-config --version >/dev/null 2>&1 			\
	|| (echo glib not found; exit 1)
	@echo TODO verify version is greater than 1.1.??

_GTK_:
	@gtk-config --version >/dev/null 2>&1			\
	|| (echo gtk not found; exit 1)
	@echo TODO verify version is greater than 1.1.??

_ZLIB_:
	@if [    -e /usr/lib/libz.a				\
	      -o -e /usr/local/lib/libz.a			\
	      -o -e /usr/X11R6/lib/libz.a			\
	    ]; then						\
		echo Found zlib in system directories;		\
	elif [ -d ../../$(ABI_DEPTH)/zlib ]; then		\
		echo Found zlib in peer directory;		\
		$(MAKE) -C ../../$(ABI_DEPTH)/zlib;		\
	else							\
		echo zlib not found;				\
		echo TODO provide a nicer error message;	\
		echo TODO with instructions for obtaining it;	\
		exit 1;						\
	fi

_LIBPNG_:
	@if [    -e /usr/lib/libpng.a				\
	      -o -e /usr/local/lib/libpng.a			\
	      -o -e /usr/X11R6/lib/libpng.a			\
	    ]; then						\
		echo Found libpng in system directories;	\
	elif [ -d ../../$(ABI_DEPTH)/libpng ]; then		\
		echo Found libpng in peer directory;		\
		$(MAKE) -C ../../$(ABI_DEPTH)/libpng;		\
	else							\
		echo libpng not found;				\
		echo TODO provide a nicer error message;	\
		echo TODO with instructions for obtaining it;	\
		exit 1;						\
	fi

_FONTS_:
	@if [ !   -d $(ABI_DEPTH)/wp/lib/unix/fonts							\
	     -o ! -f $(ABI_DEPTH)/wp/lib/unix/fonts/fonts.dir				\
		];																\
	then																\
		echo Missing font directory.;									\
		exit 1;															\
	else																\
		xxx=`grep \.pfa $(ABI_DEPTH)/wp/lib/unix/fonts/fonts.dir | awk '{print $$1}'`;	\
		for k in $$xxx;													\
		do																\
			if [ ! -f $(ABI_DEPTH)/wp/lib/unix/fonts/$$k ];				\
			then														\
				echo ***AbiWord FONT NOT FOUND***;						\
				echo;													\
				echo AbiWord has a collection of font metrics that;		\
				echo need to be downloaded separately.  Please see;		\
				echo http://www.abisource.com/dev_download.phtml#type1;	\
				echo for information.;									\
				echo;													\
				echo ***STOP***;										\
				exit 1;													\
			fi;															\
		done;															\
		echo AbiWord fonts found.;										\
	fi

_COLE_:
	@if [    -e /usr/lib/libcole.a				\
	      -o -e /usr/local/lib/libcole.a			\
	      -o -e /usr/X11R6/lib/libcole.a			\
	    ]; then						\
		echo Found libcole in system directories;	\
	elif [ -d ../../$(ABI_DEPTH)/cole-0.2.0 ]; then		\
		echo Found libcole in peer directory;		\
		pushd ../../$(ABI_DEPTH)/cole-0.2.0;		\
			./configure;				\
		popd;						\
		$(MAKE) -C ../../$(ABI_DEPTH)/cole-0.2.0;	\
	else							\
		echo libcole not found;				\
		echo ***COLE LIBRARY NOT FOUND***;		\
		echo;						\
		echo "COLE (OLE 2 structured streams library) can be obtained at"; \
		echo "http://arturo.directmail.org/filters/";	\
		echo;						\
		echo ***STOP***;				\
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

clean_COLE_:
	@if [ -d ../../$(ABI_DEPTH)/cole-0.2.0 ]; then		\
		$(MAKE) -C ../../$(ABI_DEPTH)/cole-0.2.0 clean;	\
	fi

clean_FONTS_:
	@rm -f $(ABI_DEPTH)/wp/lib/unix/fonts/*.pfa
	@rm -f $(ABI_DEPTH)/wp/lib/unix/fonts/*.afm
