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

REQUIRE=		_GCC_ _GLIB_ _GTK_ _ZLIB_ _LIBPNG_ _LIBWV_ _UNIXFONTS_ _ABIDISTFILES_ _GNOME_ _LIBICONV_
REQUIREABICLEAN=	
REQUIRECLEAN=		clean_ZLIB_ clean_LIBPNG_ clean_LIBWV_ clean_LIBICONV_
REQUIREREALCLEAN=	$(REQUIRECLEAN) 

TARGET=		build

include $(ABI_ROOT)/src/config/abi_rules.mk


build::		$(REQUIRE)
	@echo Build requirements satisfied.

abiclean:: 	$(REQUIREABICLEAN)

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
	@$(GLIB_CONFIG) --version 2>&1 1>/dev/null		\
	|| (echo glib not found; exit 1)

_GTK_:
	@$(GTK_CONFIG) --version 2>&1 1>/dev/null		\
	|| (echo gtk not found; exit 1)

# For right now GNOME is, by default, disabled, but can be forced on 
# through ABI_OPT_GNOME=1

ifdef ABI_OPT_GNOME
_GNOME_:
	@$(GNOME_CONFIG) --version 2>&1 1>/dev/null		\
	|| (echo gnome not found; exit 1)
else
_GNOME_:
endif


# zlib can only be used if located in a system directory.

_ZLIB_:
	@if [    -r /usr/lib/libz.a				\
	      -o -r ${LOCALBASE}/lib/libz.a			\
	      -o -r /usr/local/lib/libz.a			\
	      -o -r ${X11BASE}/lib/libz.a			\
	      -o -r /usr/X11R6/lib/libz.a			\
	    ]; then						\
		echo Found zlib in system directories;		\
	else							\
		echo zlib not found;				\
                echo zlib must be installed as a system library to compile AbiWord;       \
                echo Instructions for obtaining it can be found at;             \
                echo http://www.abisource.com/dev_download.phtml;               \
		exit 1;						\
	fi

# libpng can only be used if located in a system directory.

_LIBPNG_:
	@if [    -r /usr/lib/libpng.a				\
	      -o -r ${LOCALBASE}/lib/libpng.a			\
	      -o -r /usr/local/lib/libpng.a			\
	      -o -r ${X11BASE}/lib/libpng.a			\
	      -o -r /usr/X11R6/lib/libpng.a			\
	    ]; then						\
		echo Found libpng in system directories;	\
	else							\
		echo libpng not found;				\
                echo libpng must be installed as a system library to compile AbiWord.;   \
                echo Instructions for obtaining it can be found at;             \
                echo http://www.abisource.com/dev_download.phtml                \
		exit 1;						\
	fi

# Right now, we can only build wv as a peer, not in the
# system directories.

_LIBWV_:
	@if [ -r $(ABI_ROOT)/../wv ]; then			\
		echo Found libwv in peer directory;		\
		if [ ! -r $(ABI_ROOT)/../wv/config.h ]; then	\
			cd $(ABI_ROOT)/../wv; 			\
			./configure;				\
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
_UNIXFONTS_:
	@if [ -d $(ABI_ROOT)/../unixfonts ]; then		\
		echo Found unixfonts in peer directory;		\
	else							\
		echo ;						\
		echo " - unixfonts not found";			\
		echo "*** You will not be able to use the 'make install'";	\
		echo "*** target without unixfonts available as a peer";		\
		echo "*** to the abi/ tree.";			\
		echo ;						\
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

ifdef __FreeBSD__
		@if [ -d $(ABI_ROOT)/../libiconv ]; then                        	\
			echo Found libiconv in peer directory;                  \
			if [ ! -r $(ABI_ROOT)/../libiconv/src/config.h ]; then  \
				cd $(ABI_ROOT)/../libiconv;                     \
				./configure;                                    \
			fi;                                                     \
			$(MAKE) -f Makefile.abi -C $(ABI_ROOT)/../libiconv;     \
		else                                                            \
			echo libiconv not found;                                \
			echo TODO provide a nicer error message;                \
			echo TODO with instructions for obtaining it;           \
			exit 1;                                                 \
		fi								

else
	@if [ ! -r /usr/include/iconv.h -a ! -r /usr/local/include/iconv.h ]; then	\
		if [ -d $(ABI_ROOT)/../libiconv ]; then                        	\
			echo Found libiconv in peer directory;                  \
			if [ ! -r $(ABI_ROOT)/../libiconv/src/config.h ]; then  \
				cd $(ABI_ROOT)/../libiconv;                     \
				./configure;                                    \
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
endif

#################################################################

clean_ZLIB_:
	@if [ -d $(ABI_ROOT)/../zlib ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../zlib clean;	\
	fi

clean_LIBPNG_:
	@if [ -d $(ABI_ROOT)/../libpng ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../libpng clean;	\
	fi

clean_LIBWV_:
	@if [ -d $(ABI_ROOT)/../wv ]; then		\
		$(MAKE) -C $(ABI_ROOT)/../wv clean;	\

clean_LIBICONV_:
	@if [ -d $(ABI_ROOT)/../libiconv ]; then              \
		$(MAKE) -C $(ABI_ROOT)/../libiconv clean;     \

