#! gmake
 
##
## The contents of this file are subject to the AbiSource Public
## License Version 1.0 (the "License"); you may not use this file
## except in compliance with the License. You may obtain a copy
## of the License at http://www.abisource.com/LICENSE/ 
## 
## Software distributed under the License is distributed on an
## "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
## implied. See the License for the specific language governing
## rights and limitations under the License. 
## 
## The Original Code is AbiWord.
## 
## The Initial Developer of the Original Code is AbiSource, Inc.
## Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
## All Rights Reserved. 
## 
## Contributor(s):
##  

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

# note: the Linux linker is fussy about this ordering

LoadOrder=		AbiGr AbiFmt AbiDocStrux AbiImpExp AbiPS AbiAp AbiEv AbiUtil
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

