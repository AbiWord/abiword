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

include $(ABI_ROOT)/src/config/abi_defs_wp.mk

CPPSRCS=		Win32Main.cpp
RCSRCS=			Win32Main.rc

PROGRAM=		$(BINDIR)/AbiWord.exe


ifeq ($(OS_NAME), MINGW32)
# Mingw provides libiconv, a darn good one (:
ifneq ($(ABI_REQUIRE_PEER_ICONV),0)
ABIICONV = Abi_libiconv
else
ABIICONV =
endif
MINGRES_INCS=--include-dir=../../ap/win --include-dir=../../../af/xap/win --include-dir=../../../af/util/win
CPPSRCS+=		dummy.cpp

ABI_LIBS=	kernel32 user32 gdi32 winspool comdlg32 advapi32 \
			shell32 uuid comctl32 ole32 version

ifeq ($(ABI_OPT_UNICODE),1)
ABI_LIBS += unicows
endif			

#LoadOrder=	AbiGr AbiFmt AbiPTbl AbiImpExp AbiXAp AbiWPAp \
#			AbiEv AbiUtil AbiImpExp AbiWordVer AbiSpell Abi_libwv fribidi \
#			Abi_libpopt $(ABIICONV) Abi_libpng Abi_zlib
LoadOrder=	AbiGr AbiFmt AbiXAp AbiWPAp AbiPTbl AbiImpExp \
			AbiEv AbiUtil AbiGr AbiWordVer AbiSpell 


ABI_PERIPHLIBS=		fribidi \
			Abi_libpopt $(ABIICONV) Abi_libpng
ifeq ($(HAVE_WV_PEER),1)
ABI_PERIPHLIBS+=	Abi_libwv 
endif

ifeq ($(ABI_OPT_PEER_EXPAT), 1)
ABI_PERIPHLIBS+=	Abi_libexpat
endif

ABI_APPLIBS=		$(LoadOrder) $(LoadOrder) $(LoadOrder) $(ABI_PERIPHLIBS)
ABI_APPLIBDEP=		$(LoadOrder)

EXE_LDFLAGS =  -Wl,--gc-sections -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--out-implib,libAbiWord.exe.a
endif


TARGETS=		$(PROGRAM)

include $(ABI_ROOT)/src/config/abi_rules.mk

##################################################################
## everything below here must *FOLLOW* abi_rules
##################################################################

ifeq ($(OS_NAME), MINGW32)
$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP)
	@$(MAKE_OBJDIR)
ifeq ($(ABI_OPT_WINELIB), 1)
	$(CCC) -o $@ $(CFLAGS) $(OBJDIR)/Win32Main.o $(OBJDIR)/Win32Mainrc.o $(LDFLAGS) $(OBJDIR)/dummy.o $(EXTRA_LIBS) $(OS_LIBS) $(ABI_ZLIB_LIB) $(XLDFLAGS)
else
	@echo Linking dynamic $(PROGRAM)
	$(CCC) -o $@ $(CFLAGS)$(EXE_LDFLAGS) $(OBJDIR)/Win32Main.o $(OBJDIR)/Win32Mainrc.o $(LDFLAGS) -L$(LIBDIR) $(OS_LIBS) $(ABI_ZLIB_LIB) $(XLDFLAGS) $(EXTRA_LIBS) $(OS_LIBS) $(ABI_ZLIB_LIB)
	@mv libAbiWord.exe.a $(LIBDIR)
endif
else
$(PROGRAM): $(OBJS) $(EXTRA_LIBDEP)
	@$(MAKE_OBJDIR)
	@echo Linking $(PROGRAM)....
	@$(CC) $(shell echo $(OBJS) | $(TRANSFORM_TO_DOS_PATH))	\
		-Fe$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH))	\
		-link $(LDFLAGS) $(OS_LIBS) $(ABI_ZLIB_LIB) $(ABI_LDFLAGS)				\
		$(shell echo $(EXTRA_LIBS) | $(TRANSFORM_TO_DOS_PATH))
endif

build:: $(TARGETS)
	@echo Target $(TARGETS) completed.
