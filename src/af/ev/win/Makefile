#! gmake

## AbiSource Program Utilities
## Copyright (C) 1998 AbiSource, Inc.
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Library General Public
## License as published by the Free Software Foundation; either
## version 2 of the License, or (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Library General Public License for more details.
##
## You should have received a copy of the GNU Library General Public
## License along with this library; if not, write to the
## Free Software Foundation, Inc., 59 Temple Place - Suite 330,
## Boston, MA 02111-1307, USA.

ABI_DEPTH = ../..

include $(ABI_DEPTH)/config/abi_defs.mk

## TODO: decide if all of these need to be exported.
## TODO: move the ones which dont to the HEADERS definition

EXPORT_HEADERS=		ev_Win32Keyboard.h		\
			ev_Win32Mouse.h

HEADERS=		$(EXPORT_HEADERS)

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		ev_Win32Keyboard.cpp		\
			ev_Win32Mouse.cpp


INCLUDES=		-I$(DIST)/include -I. -DUT_DEBUG

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

export:: $(EXPORT_HEADERS)
	@echo Exporting headers from `pwd`...
	@$(subst xxxx,$(DIST)/include,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(EXPORT_HEADERS) $(DIST)/include

libs:: $(TARGETS)
