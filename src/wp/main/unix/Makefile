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

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

## TODO: decide if all of these need to be exported.
## TODO: move the ones which don't to the HEADERS definition

EXPORT_HEADERS=
HEADERS=		$(EXPORT_HEADERS)


RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		UnixMain.cpp

INCLUDES=		-I$(DIST)/include -I. -I../xp `gtk-config --cflags`

PROGRAM=		AbiWord

# TODO the Linux linker is fussy about the ordering of libraries.
# TODO in order to get all symbols to resolve, i had to list them
# TODO several times.  see if there is a better way of doing this.

LoadOrder=		AbiGr AbiFmt AbiPTbl AbiImpExp AbiPS AbiAp AbiEv AbiUtil
ABI_APPLIBS=		$(LoadOrder) $(LoadOrder) $(LoadOrder)
ABI_OTHLIBS=		expat 
ABI_LIBS=		stdc++

TARGETS=		$(PROGRAM)

include $(ABI_DEPTH)/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

OBJS +=			$(OTHER_OBJS)

## TODO decide how/if to build completely statically.
## TODO defer decision until later.
## TODO you'll need to set LD_LIBRARY_PATH to $(DIST)/bin
## TODO in your environment in order to run the executable

##LDFLAGS +=		-static
##LDFLAGS +=		-shared

# we override the $(PROGRAM) rule.

$(PROGRAM): $(OBJS)
	@$(MAKE_OBJDIR)
	$(CCC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) $(EXTRA_LIBS)

ifneq ($(strip $(EXPORT_HEADERS)),)
export:: $(EXPORT_HEADERS)
	$(subst xxxx,$(DIST)/include,$(VERIFY_DIRECTORY))
	$(INSTALL) -m 444 $(EXPORT_HEADERS) $(DIST)/include
endif

install:: $(TARGETS)
	$(subst xxxx,$(DIST)/bin,$(VERIFY_DIRECTORY))
	$(INSTALL) -m 755 $(TARGETS) $(DIST)/bin

##################################################################

$(OTHER_OBJS)::
	cd ../xp; $(MAKE)

