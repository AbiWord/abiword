#! gmake

## AbiSource Setup Kit
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

CSRCS=			setup.c

GENCSRCS=		$(OBJDIR)/data.c

RCSRCS=			setup.rc

PROGRAM=		$(BINDIR)/setup_abiword.exe

ABI_LDFLAGS=		/nologo /subsystem:windows /incremental:no /machine:$(LINK_ARCH) /profile
ABI_APPLIBS=		AbiSetup Abi_zlib Abi_libpng

ABI_OTHLIBS=		
####ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
####			shell32 ole32 oleaut32 uuid odbc32 odbccp32
ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
			shell32 uuid comctl32 ole32

GARBAGE=		setup.map setup.pdb $(OBJDIR)/data.c _tmp_.ask
TARGETS=		$(PROGRAM)

INCLUDES=		-I$(ABI_XX_ROOT)/src/ask/lib/xp

CFLAGS +=		-Zm1000

include $(ABI_ROOT)/src/config/abi_rules.mk

ifdef ABISOURCE_LICENSED_TRADEMARKS
GRAPHICNAME=setup_tm.png
else
GRAPHICNAME=setup.png
endif

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

$(OBJDIR)/data.c:	abisuite.ask $(BINDIR)/ask_gendata.exe $(BINDIR)/askrm.exe
	rm -f $(OBJDIR)/_tmp_.ask
	sed -e 's|__CANONDIR__|$(shell echo $(CANONDIR) | $(TRANSFORM_TO_DOS_PATH) | sed 's|\\\\|/|g')|g' \
	  abisuite.ask > $(OBJDIR)/_tmp1_.ask
	sed -e 's|__BINDIR__|$(shell echo $(BINDIR) | $(TRANSFORM_TO_DOS_PATH) | sed 's|\\\\|/|g')|g' \
	  $(OBJDIR)/_tmp1_.ask > $(OBJDIR)/_tmp2_.ask
	sed -e 's|__GRAPHICNAME__|$(shell echo $(GRAPHICNAME) | $(TRANSFORM_TO_DOS_PATH) | sed 's|\\\\|/|g')|g' \
	  $(OBJDIR)/_tmp2_.ask > $(OBJDIR)/_tmp3_.ask
	$(BINDIR)/ask_gendata.exe \
	     $(shell echo $(OBJDIR)/_tmp3_.ask | $(TRANSFORM_TO_DOS_PATH)) \
		 $(shell echo $(OBJDIR)/_tmp_.c | $(TRANSFORM_TO_DOS_PATH))
	(cd $(OBJDIR); mv _tmp_.c data.c)

$(PROGRAM): $(OBJS)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	@$(CC) $(shell echo $(OBJS) | $(TRANSFORM_TO_DOS_PATH)) -Fe$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH)) -link $(LDFLAGS) $(OS_LIBS) $(ABI_LDFLAGS) $(shell echo $(EXTRA_LIBS) | $(TRANSFORM_TO_DOS_PATH))

ifdef ABI_OPT_DEBUG

distribution::
	@echo ABI_OPT_DEBUG is set.  Building a setup program in
	@echo debug mode is not allowed.
	exit 1

else

distribution:: $(TARGETS)
	@echo Copying $(PROGRAM) to $(DIST)
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(PROGRAM) $(DIST)

endif
