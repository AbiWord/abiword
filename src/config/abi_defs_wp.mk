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

##################################################################
## Deal with branding issues.

ifdef ABISOURCE_LICENSED_TRADEMARKS

ifeq ($(OS_NAME),WIN32)
ABI_NAMEDEFS=	-DABIWORD_APP_NAME=\"\"AbiWord\"\"
else
ABI_NAMEDEFS=	-DABIWORD_APP_NAME="\"AbiWord\""
endif

else

ifeq ($(OS_NAME),WIN32)
ABI_NAMEDEFS=	-DABIWORD_APP_NAME=\"\"AbiWord Personal\"\"
else
ABI_NAMEDEFS=	-DABIWORD_APP_NAME="\"AbiWord Personal\""
endif

endif

##################################################################

