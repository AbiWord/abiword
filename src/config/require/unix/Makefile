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

REQUIRE=		_GCC_ _GLIB_ _GTK_ _ZLIB_ _LIBPNG_ 
## REQUIRE=		_GCC_ _GLIB_ _GTK_ _ZLIB_ _LIBPNG_ #_COLE_
REQUIRECLEAN=		clean_ZLIB_ clean_LIBPNG_ #clean_COLE_
REQUIREREALCLEAN=	$(REQUIRECLEAN) 

TARGET=		build

include $(ABI_ROOT)/src/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

clean::		$(REQUIRECLEAN)

realclean::	$(REQUIREREALCLEAN)

#################################################################

_GCC_:
	@echo TODO do we need to test for GCC or EGCS and a version number?

REQ_GTK_MINOR=2
REQ_GTK_BUILD=1

# FreeBSD ports versions of glib and gtk name things like "glib12-config"
# and "gtk12-config", so we check for those also.  Thanks to 
# Ming-I Hsieh <mihs@wm28.csie.ncu.edu.tw> for the patch.

_GLIB_:
	@glib-config --version >/dev/null 2>&1 			\
	|| @glib12-config --version >/dev/null 2>&1 			\
	|| (echo glib not found; exit 1)
	@echo TODO verify glib version is greater than 1.1.??

_GTK_:
	@gtk-config --version >/dev/null 2>&1			\
	|| @gtk12-config --version >/dev/null 2>&1			\
	|| (echo gtk not found; exit 1)
	@echo TODO verify gtk version is greater than 1.1.??

_ZLIB_:
	@if [    -r /usr/lib/libz.a				\
	      -o -r /usr/local/lib/libz.a			\
	      -o -r /usr/X11R6/lib/libz.a			\
	    ]; then						\
		echo Found zlib in system directories;		\
	elif [ -d $(ABI_ROOT)/../zlib ]; then			\
		echo Found zlib in peer directory;		\
		$(MAKE) -C $(ABI_ROOT)/../zlib;			\
	else							\
		echo zlib not found;				\
		echo TODO provide a nicer error message;	\
		echo TODO with instructions for obtaining it;	\
		exit 1;						\
	fi

_LIBPNG_:
	@if [    -r /usr/lib/libpng.a				\
	      -o -r /usr/local/lib/libpng.a			\
	      -o -r /usr/X11R6/lib/libpng.a			\
	    ]; then						\
		echo Found libpng in system directories;	\
	elif [ -d $(ABI_ROOT)/../libpng ]; then			\
		echo Found libpng in peer directory;		\
		$(MAKE) -C $(ABI_ROOT)/../libpng;		\
	else							\
		echo libpng not found;				\
		echo TODO provide a nicer error message;	\
		echo TODO with instructions for obtaining it;	\
		exit 1;						\
	fi

_COLE_:
	@if [    -r /usr/lib/libcole.a				\
	      -o -r /usr/local/lib/libcole.a			\
	      -o -r /usr/X11R6/lib/libcole.a			\
	    ]; then						\
		echo Found libcole in system directories;	\
	elif [ -d $(ABI_ROOT)/../cole-0.2.0 ]; then		\
		echo Found libcole in peer directory;		\
		pushd $(ABI_ROOT)/../cole-0.2.0;		\
			./configure;				\
		popd;						\
		$(MAKE) -C $(ABI_ROOT)/../cole-0.2.0;		\
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
	@if [ -d $(ABI_ROOT)/../zlib ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../zlib clean;	\
	fi

clean_LIBPNG_:
	@if [ -d $(ABI_ROOT)/../libpng ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../libpng clean;	\
	fi

clean_COLE_:
	@if [ -d $(ABI_ROOT)/../cole-0.2.0 ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../cole-0.2.0 clean;	\
	fi
