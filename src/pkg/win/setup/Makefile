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

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

CSRCS=			setup.c			\
			$(OBJDIR)/data.c

PROGRAM=		$(BINDIR)/setup.exe

ABI_LDFLAGS=		/nologo /subsystem:windows /incremental:no /machine:I386 /profile
ABI_APPLIBS=		AbiSetup Abi_zlib Abi_libpng

ABI_OTHLIBS=		
####ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
####			shell32 ole32 oleaut32 uuid odbc32 odbccp32
ABI_LIBS=		kernel32 user32 gdi32 winspool comdlg32 advapi32 \
			shell32 uuid comctl32 ole32

GARBAGE=		setup.map setup.pdb $(OBJDIR)/data.c _tmp_.ask
TARGETS=		$(PROGRAM)

INCLUDES=		-I$(ABI_DEPTH)/ask/lib/xp

CFLAGS +=		-Zm1000

include $(ABI_DEPTH)/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

$(OBJDIR)/data.c:	abisuite.ask $(BINDIR)/ask_gendata.exe $(BINDIR)/askrm.exe
	rm -f $(OBJDIR)/_tmp_.ask
	sed -e 's|__CANONDIR__|$(CANONDIR)|g' abisuite.ask > $(OBJDIR)/_tmp1_.ask
	sed -e 's|__BINDIR__|$(BINDIR)|g' $(OBJDIR)/_tmp1_.ask > $(OBJDIR)/_tmp_.ask
	$(BINDIR)/ask_gendata.exe $(OBJDIR)/_tmp_.ask $(OBJDIR)/_tmp_.c
	mv $(OBJDIR)/_tmp_.c $(OBJDIR)/data.c

$(PROGRAM): $(OBJS)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	@$(CC) $(subst /,\\,$(OBJS)) -Fe$(subst /,\\,$@) -link $(LDFLAGS) $(OS_LIBS) $(ABI_LDFLAGS) $(subst /,\\,$(EXTRA_LIBS))

ifdef ABI_OPT_DEBUG

distribution::
	@echo ABI_OPT_DEBUG is set.  Building a setup program in
	@echo debug mode is not allowed.

else

distribution:: $(TARGETS)
	@echo Copying $(PROGRAM) to $(DIST)
	@$(subst xxxx,$(DIST),$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(PROGRAM) $(DIST)

endif
