#! gmake

## AbiSource Applications
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

DEPENDS=

ifdef ABI_OPT_JS
DEPENDS+=		_JS_
endif

DEPENDS_ABIWORD=	$(DEPENDS)
DEPENDS_ABICALC=	$(DEPENDS) _GLIB_ _GXML_

all:		$(DEPENDS_ABIWORD)
	@echo Building AbiWord and AbiCalc...
	$(MAKE) -C src T=all  &&  echo AbiWord and AbiCalc build complete.

abiword:	$(DEPENDS_ABIWORD)
	@echo Building AbiWord...
	$(MAKE) -C src T=abiword  &&  echo AbiWord build complete.

abicalc:	$(DEPENDS_ABICALC)
	@echo Building AbiCalc...
	$(MAKE) -C src T=abicalc  &&  echo AbiCalc build complete.


## phony targets to build the 3rd-party libraries that we need.

_JS_:
	@echo Building JavaScript...
	sh BUILD_JS.sh  &&  echo JavaScript build complete.

_GXML_:
	@echo Building GXML...
	sh BUILD_GXML.sh  &&  echo GXML build complete.

_GLIB_:
	@echo Building GLIB...
	sh BUILD_GLIB.sh  &&  echo GLIB build complete.
