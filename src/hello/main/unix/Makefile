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

include $(ABI_DEPTH)/config/abi_defs_he.mk

CPPSRCS=		UnixMain.cpp

INCLUDES=		`gtk-config --cflags`

PROGRAM=		AbiHello

# TODO the Linux linker is fussy about the ordering of libraries.
# TODO in order to get all symbols to resolve, i had to list them
# TODO several times.  see if there is a better way of doing this.

LoadOrder=		AbiGr AbiHEAp AbiXAp AbiEv AbiUtil AbiVer
ABI_APPLIBS=		$(LoadOrder) $(LoadOrder) $(LoadOrder)

ABI_OTHLIBS=		expat $(ABI_JSLIBS) $(DL_LIBS)
ABI_LIBS=		stdc++

TARGETS=		$(PROGRAM)

include $(ABI_DEPTH)/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

## TODO decide how/if to build completely statically.
## TODO defer decision until later.
## TODO you'll need to set LD_LIBRARY_PATH to $(DIST)/bin
## TODO in your environment in order to run the executable

##LDFLAGS +=		-static
##LDFLAGS +=		-shared

# ACK!  TODO - do we really want a directory pattern match here in a makefile?
# 	TODO - using $(shell) is just slightly better than letting the compiler
#              do the expansion

THE_LIBS=		$(shell ls $(DIST)/lib/*)

# we override the $(PROGRAM) rule.

$(PROGRAM): $(OBJS) $(THE_LIBS)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	@$(CCC) -o $@ $(CFLAGS) $(OBJS) -L/usr/local/lib $(LDFLAGS) $(EXTRA_LIBS) $(XLDFLAGS) -L/usr/local/lib

build:: $(TARGETS)
	@echo Exporting $(TARGETS) from `pwd`
	@$(subst xxxx,$(DIST)/bin,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(TARGETS) $(DIST)/bin



