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

##############################################################################
## Win32 platform defines
##############################################################################

##################################################################
##################################################################
## The main makefile and/or this file requires that OS_ARCH be set
## to something to describe which chip that this OS is running on.
## This can be used to change which tools are used and/or which
## compiler/loader options are used.  It will probably also be used
## in constructing the name object file destination directory.

ifndef OS_ARCH
OS_ARCH		:= $(shell uname -m | sed -e s/i.86/i386/ | sed "s/\//-/")
endif

# remaining portion of object directory, e.g.  WIN32_1.3.9_i386_VC_OBJ
ifeq ($(ABI_OPT_DEBUG),1)
OBJ_DIR_SFX	+= $(CCSET)_DBG
else
OBJ_DIR_SFX	+= $(CCSET)_OBJ
endif

##################################################################
##################################################################
## Define tools, ie compiler, linker, etc
## The primary compiler is (MSVC) Microsoft's Visual C/C++
## however we also provide limited support [may require fixups]
## for other toolchains which may be selected via CCSET

## Supported compiler sets are:
## VC   - MSVC 5 or 6 compatible tools
## VC7  - MSVC 7 or 7.1 compatible tools
## EVC3 - Embedded Visual C++ version 3
## OW   - Open Watcom 1.2 compatible tools
## BC5  - Borland 5.5 free cmd line tools or compatible
## DM   - Digital Mars
## for gcc, see MinGW or CygWin builds

## Default should match compiler used for release builds
## which presently is Microsoft Visual C++ 6 [optimized]
ifndef CCSET
CCSET := VC
endif
ABI_OPTIONS += CCSET=$(CCSET)


RANLIB 	= echo
BSDECHO	= echo
RCFLAGS =

# Suffixes
OBJ_SUFFIX := obj
LIB_SUFFIX := lib
DLL_SUFFIX := dll
EXE_SUFFIX := .exe


# Architecture-specific flags
ifeq ($(OS_ARCH), i386)
ARCH_FLAGS	= -D_X86_
LINK_ARCH	= IX86
OS_ENDIAN	= LittleEndian32
endif

#ifeq ($(OS_ARCH), alpha)
#ARCH_FLAGS	= -D_ALPHA_
#ifndef OPTIMIZER
#OPTIMIZER	= -Oib3 -QAtuneEV56 -QAarchEV4 -QAgq -QAOu0 -QAieee0
#endif
#LINK_ARCH	= ALPHA
#OS_ENDIAN	= LittleEndian32
#endif


##################################################################
# Compiler specific settings

# MSVC 5+
ifeq ($(subst VC7,VC,$(CCSET)), VC)
CC 	= cl
CCC 	= cl
LINK 	= link
AR 	= lib -NOLOGO -OUT:"$(shell echo $@ | sed 's|/cygdrive/\([a-zA-Z]\)/|\1:/|g' | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')"
RC 	= rc.exe $(RCFLAGS) 

# Compiler and shared library flags 

# set optimization flags, but only if not already set
ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER 	?= -Od -Ob1
DEFINES 	= -DDEBUG -D_DEBUG -UNDEBUG -D_CRTDBG_MAP_ALLOC
OS_CFLAGS 	= -MDd -Z7
else
OPTIMIZER	?= -O2
DEFINES	= -UDEBUG -U_DEBUG -DNDEBUG
OS_CFLAGS 	= -MD -Zi
endif

OS_CFLAGS 	+= -W3 -nologo -GF -Gy -GX -Zm200
# -W# warning level, 3=production level, 4=lint level, 0=disable all warnings
# -nologo suppresses banner
# -GF & -Gf enables string pooling (/GF places them in read-only memory)
# -Gy enables function level linking
# -GX enable [synchronous] exception handling
# -MD to use MSVCRT.DLL and -MDd to use debug variant MSVCRTd.DLL,
#     for multithreaded DLL c runtime library
# -Zm# increases comilers internal heap allocations by # percent, ie 200=twice defaut size
# -Zi create debug info, store in .pdb file; -ZI for edit & continue; -Z7 for older embedded
# -O? optimize, -Od disable; -Ob1 expand only inline marked items, -Ob2 compiler
#     decides what to inline; -Og global; -Oi enable intrinsics; -Ot favor small code; -Oy omit
#     frame pointer, use -Oy- after -O2 to force frame pointers
#     -O2 == /0g /Oi /Ot /Oy /Ob1 /Gs /Gf /Gy
DLLFLAGS 	= -OUT:"$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH))" 
LDFLAGS 	= -DEBUG -MACHINE:$(LINK_ARCH) -MAPINFO:LINES -MAP -LIBPATH:$(shell echo $(LIBDIR) | $(TRANSFORM_TO_DOS_PATH) )
OS_DLLFLAGS = -nologo -DLL -SUBSYSTEM:WINDOWS -DEBUG -MACHINE:$(LINK_ARCH)

endif

# MSVC 7+
# use a few VC 7 specific additions, but otherwise same as VC5+
ifeq ($(CCSET), VC7)

ifeq ($(ABI_OPT_DEBUG),1)
OS_CFLAGS 	+= -RTC1 -GS
else
OPTIMIZER	+= -GL
LDFLAGS	+= -LTCG
endif
LDFLAGS 	+= -OPT:REF,ICF

endif

# EVC3 - Embedded Visual C++ version 3
ifeq ($(CCSET), EVC3)

endif


# OW   - Open Watcom 1.2 compatible tools
ifeq ($(CCSET), OW)

CC 	= cl386
CCC 	= cl386
LINK 	= link
AR 	= lib -NOLOGO -OUT:"$(shell echo $@ | sed 's|/cygdrive/\([a-zA-Z]\)/|\1:/|g' | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')"
RC 	= rc $(RCFLAGS) 

# set optimization flags, but only if not already set
ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER 	?= -Od -Ob1
DEFINES 	= -DDEBUG -D_DEBUG -UNDEBUG
OS_CFLAGS 	= -MDd -Zi
else
OPTIMIZER	?= -O2
DEFINES	= -UDEBUG -U_DEBUG -DNDEBUG
OS_CFLAGS 	= -MD -Zi
endif

OS_CFLAGS 	+= -W3 -nologo -GF -Gy -GX -Zm200
DLLFLAGS 	= -OUT:"$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH))" 
LDFLAGS 	= -DEBUG -MACHINE:$(LINK_ARCH) -MAPINFO:LINES -MAP
OS_DLLFLAGS = -nologo -DLL -SUBSYSTEM:WINDOWS -DEBUG -MACHINE:$(LINK_ARCH)

endif


# BC5  - Borland 5.5 free cmd line tools or compatible
ifeq ($(CCSET), BC5)

CC 	= bcc32
CCC 	= bcc32
LINK 	= ilink32
AR 	= tlib -NOLOGO -OUT:"$(shell echo $@ | sed 's|/cygdrive/\([a-zA-Z]\)/|\1:/|g' | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')"
RC 	= brc32 $(RCFLAGS) 

# set optimization flags, but only if not already set
ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER 	?= -Od
DEFINES 	= -DDEBUG -D_DEBUG -UNDEBUG
OS_CFLAGS 	= -v
else
OPTIMIZER	?= -O2 -4
DEFINES	= -UDEBUG -U_DEBUG -DNDEBUG
OS_CFLAGS 	= 
endif

OS_CFLAGS 	+= -N -d
DLLFLAGS 	= -OUT:"$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH))" 
LDFLAGS 	= -DEBUG -MACHINE:$(LINK_ARCH) -MAPINFO:LINES -MAP
OS_DLLFLAGS = -nologo -DLL -SUBSYSTEM:WINDOWS -DEBUG -MACHINE:$(LINK_ARCH)

endif


# DM   - Digital Mars
ifeq ($(CCSET), DM)

CC 	= sc
CCC 	= sc
LINK 	= link
AR 	= lib -NOLOGO -OUT:"$(shell echo $@ | sed 's|/cygdrive/\([a-zA-Z]\)/|\1:/|g' | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')"
RC 	= rcc $(RCFLAGS) 

# set optimization flags, but only if not already set
ifeq ($(ABI_OPT_DEBUG),1)
OPTIMIZER 	?= 
DEFINES 	= -DDEBUG -D_DEBUG -UNDEBUG
OS_CFLAGS 	= 
else
OPTIMIZER	?= -O2 -4
DEFINES	= -UDEBUG -U_DEBUG -DNDEBUG
OS_CFLAGS 	= 
endif

OS_CFLAGS 	+= -N -d
DLLFLAGS 	= -OUT:"$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH))" 
LDFLAGS 	= -DEBUG -MACHINE:$(LINK_ARCH) -MAPINFO:LINES -MAP
OS_DLLFLAGS = -nologo -DLL -SUBSYSTEM:WINDOWS -DEBUG -MACHINE:$(LINK_ARCH)

endif


##################################################################
# libraries and other compile/link settings

DEFINES 	+= -DWIN32 -DSUPPORTS_UT_IDLE

# we used to always use peer iconv, but not any more as of
# CVS HEAD, 2 Jun 2006.
# ABI_REQUIRE_PEER_ICONV = 1


ifeq(ABI_REQUIRE_PEER_ICONV, 1)
 ABI_LIBS += Abi_libiconv
else
 OS_LIBS += $(ICONV_LIBS) -liconv # ICONV_LIBS is set in the environment equal to "-L/some/path" where there is /some/path/libiconv.so.  It may remain unset if the lib is already in the default search path, as it usually is.
endif

# if not specified, default to libxml2 for xml parsing 7 June 2006
ABI_OPT_PEER_EXPAT ?= 0

# add wv's mini glib to include list
ABI_OTH_INCS+=	/../../wv/glib-wv

ifndef ENABLE_UNICODE
UNICODE_CFLAGS =
UNICODE_LIBS =
else
# NOTE: Helper applications that should not be built
#       unicode enabled, MUST redefine all UNICODE_*
#       variables after including abi_defs.mk and prior
#       to performing the actual build rules.
UNICODE_CFLAGS 	= -DUNICODE -D_UNICODE

# WARNING unicows _MUST_ appear before any OS libs
# Should only be used by Unicode enabled applications
UNICODE_LIBS = \
	-nod:kernel32.lib -nod:advapi32.lib -nod:user32.lib -nod:gdi32.lib \
	-nod:shell32.lib -nod:comdlg32.lib -nod:version.lib -nod:mpr.lib \
	-nod:rasapi32.lib -nod:winmm.lib -nod:winspool.lib -nod:vfw32.lib \
	-nod:secur32.lib -nod:oleacc.lib -nod:oledlg.lib -nod:sensapi.lib \
	-nod:uuid.lib -nod:comctl32.lib -nod:ole32.lib\
              unicows.lib
endif

OS_LIBS		= \
              $(UNICODE_LIBS) \
              kernel32.lib \
              user32.lib \
              gdi32.lib \
              winspool.lib \
              comdlg32.lib \
              advapi32.lib \
              shell32.lib \
              uuid.lib \
              comctl32.lib \
              version.lib \
              ole32.lib


OS_CFLAGS 	+= $(UNICODE_CFLAGS) $(OPTIMIZER) $(ARCH_FLAGS)

ABI_NATIVE	= win
ABI_FE		= Win32

##################################################################
## ABIPKGDIR defines the directory containing the Makefile to use to
## build a set of distribution archives (.deb, .rpm, .tgz, .exe, etc)
## This is relative to $(ABI_ROOT)/src/pkg

ABIPKGDIR	= win/setup

## Default path for NSIS v2
NSIS_ROOT ?= "/Program Files/NSIS"

##################################################################
## EXPAT_PLATFORM_DEFS and PSICONV_PLATFORM_DEFS are the flags
## that should be passed to configure when building expat and
## psiconv, respectively, for this platform (if any)
#EXPAT_PLATFORM_DEFS=CC=cl.exe CONFIG_SHELL=sh.exe CFLAGS='$(OS_CFLAGS)' 

# require'd library settings
ABI_ZLIB_ROOT = $(ABI_XX_ROOT)/../libs/zlib
#OS_LIBS += -L$(ABI_ZLIB_ROOT)/lib
ABI_ZLIB_INC = $(ABI_ZLIB_ROOT)/include
ABI_ZLIB_LIB = $(ABI_ZLIB_ROOT)/lib/zdll.lib
ABI_LIBS += $(ABI_ZLIB_ROOT)/lib/zdll

# TODO: support fribidi as DLL or statically compiled in
CFLAGS += -DFRIBIDI_EXPORTS	# symbols match
ABI_PEER_INCS  += /../../fribidi	# so <fribidi.h> works

# End of win32 defs
