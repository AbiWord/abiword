#! gmake

## AbiSource Program Utilities
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

ABI_AP_INCS=	/wp/ap/xp	/wp/ap/$(ABI_NATIVE)	\
		/text/fmt/xp				\
		/wp/impexp/xp				\
		/text/ptbl/xp	/wp/ap/xp/ToolbarIcons

include $(ABI_ROOT)/src/config/abi_defs.mk

ifdef ABI_OPT_GNOME
ABI_AP_INCS+=	/wp/ap/$(ABI_NATIVE)/$(ABI_GNOME_DIR) 
endif

##################################################################
##
## Define the name of the application.  The quoting stuff is beacuse
## of Win32 slash lossage.  
##

ifdef ABI_BIDI_ENABLED
BIDI_STRING= -BiDi
endif

ifeq ($(ABI_ESCAPE_QUOTES),YES)
ABI_NAMEDEFS=	-DABIWORD_APP_NAME=\"\"AbiWord$(BIDI_STRING)\"\"
else
ABI_NAMEDEFS=	-DABIWORD_APP_NAME="\"AbiWord$(BIDI_STRING)\""
endif

################################################################
## ABI_APPLIBDIR defines the name of the root of the app-specific
##               library directory in the canonical layout.
## ABI_APPLIBDIRDEF is the same value properly quoted for use
##                  within the application.

ABI_APPLIBDIR=AbiWord
ifeq ($(ABI_ESCAPE_QUOTES),YES)
ABI_APPLIBDIRDEF= -DABIWORD_APP_LIBDIR=\"\"AbiWord\"\"
else
ABI_APPLIBDIRDEF= -DABIWORD_APP_LIBDIR="\"AbiWord\""
endif

##################################################################

