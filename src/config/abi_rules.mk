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

##################################################################
##################################################################
## abi_rules.mk --  Makefile definitions for building AbiSource software.
## This is a makefile include.  It should be included before any
## other rules are defined.
##
## The general structure of an AbiSource Makefile should be:
##
##        #! gmake
##        ABI_ROOT=<the top-level abi directory>
##        include $(ABI_ROOT)/src/config/abi_defs.mk
##        <local declarations>
##        include $(ABI_ROOT)/src/config/abi_rules.mk
##        <local rules>
##
##################################################################
##################################################################

# These options should be either 0 (off) or 1 (on)
ifneq ($(ABI_OPT_DEBUG),1)
ABI_OPT_DEBUG=0
endif
ifneq ($(ABI_OPT_GNOME),1)
ABI_OPT_GNOME=0
endif
ifneq ($(ABI_OPT_NAUTILUS),1)
ABI_OPT_NAUTILUS=0
endif
ifneq ($(ABI_OPT_PERL),1)
ABI_OPT_PERL=0
endif
ifneq ($(ABI_OPT_LIBJPEG),1)
ABI_OPT_LIBJPEG=0
endif
ifneq ($(ABI_OPT_MSXML),1)
ABI_OPT_MSXML=0
endif
ifneq ($(ABI_OPT_PEER_EXPAT),1)
ABI_OPT_PEER_EXPAT=0
endif
ifneq ($(ABI_OPT_OPTIMIZE),1)
ABI_OPT_OPTIMIZE=0
endif
ifneq ($(ABI_OPT_PSPELL),1)
ABI_OPT_PSPELL=0
endif

ifdef LIBRARY_NAME
ifeq ($(OS_NAME), WIN32)
 LIBPREFIX	=
else
 LIBPREFIX = lib
endif
 LIBRARY		= $(LIBDIR)/$(LIBPREFIX)$(LIBRARY_NAME)$(LIBRARY_VERSION).$(LIB_SUFFIX)
 SHARED_LIBRARY		= $(LIBDIR)/$(LIBPREFIX)$(LIBRARY_NAME)$(LIBRARY_VERSION).$(DLL_SUFFIX)
 PLUGIN          	= $(PLUGINDIR)/$(LIBPREFIX)$(LIBRARY_NAME)$(LIBRARY_VERSION).$(DLL_SUFFIX)
endif

ifndef TARGETS
TARGETS		= $(LIBRARY) $(SHARED_LIBRARY)
endif

#
# OBJS is the list of object files.  It can be constructed by
# specifying CSRCS (list of C source files) and ASFILES (list
# of assembly language source files).
#

ifndef OBJS
OBJS		= $(addprefix $(OBJDIR)/,$(CSRCS:.c=.$(OBJ_SUFFIX)))		\
		  $(addprefix $(OBJDIR)/,$(CPPSRCS:.cpp=.$(OBJ_SUFFIX)))	\
		  $(addprefix $(OBJDIR)/,$(notdir $(GENCPPSRCS:.cpp=.$(OBJ_SUFFIX))))	\
		  $(addprefix $(OBJDIR)/,$(notdir $(GENCSRCS:.c=.$(OBJ_SUFFIX))))	\
		  $(addprefix $(OBJDIR)/,$(ASFILES:.s=.$(OBJ_SUFFIX)))
endif
ifndef SHORT_OBJS
SHORT_OBJS     = $(subst $(OBJDIR)/,,$(OBJS))
endif

#
# Win32 resource file
#

ifeq ($(OS_NAME), WIN32)
RCOBJS		= $(addprefix $(OBJDIR)/,$(RCSRCS:.rc=.res))
OBJS		+= $(RCOBJS)
endif
ifeq ($(OS_NAME), MINGW32)
RCOBJS		= $(addprefix $(OBJDIR)/,$(RCSRCS:.rc=rc.o))
OBJS		+= $(RCOBJS)
endif

# 
# MacOS (X) resource file
#
ifeq ($(OS_NAME), MACOSX)
REZOBJS		= $(addprefix $(OBJDIR)/,$(REZSRCS:.r=.rsrc))
OBJS		+= $(REZOBJS)
endif


#
# Trash which can be deleted
#

ALL_TRASH		= $(TARGETS) $(OBJS) $(OBJDIR) $(LIBDIR) LOGS TAGS $(GARBAGE) \
			  $(NOSUCHFILE) \
			  so_locations


ifdef DIRS
ifdef ABI_PEER
PEER_HACK	= -f Makefile.abi
else
PEER_HACK	= 
endif

LOOP_OVER_DIRS		=						\
	@for d in $(DIRS); do						\
		if test -d $$d; then					\
			set -e;						\
			echo "$(MAKE) $(PEER_HACK) ABI_ROOT=$(ABI_ROOT) -C $$d $@";	\
			$(MAKE) $(PEER_HACK) ABI_ROOT=$(ABI_ROOT) -C $$d $@;		\
			set +e;						\
		else							\
			echo "Skipping non-directory $$d...";		\
		fi;							\
	done
endif

################################################################################

all:: build

build::
	@echo Building on [$(OS_NAME)] with [$(ABI_OPTIONS)].
	+$(LOOP_OVER_DIRS)

tidy::
	rm -rf $(OBJS)
	+$(LOOP_OVER_DIRS)


# The directories used for documentation generation are listed in Doxyfile
# in the INPUT variable.
dox::
	doxygen 

################################################################################

ifdef HELPER_PROGRAM
$(HELPER_PROGRAM): $(OBJS)
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME),WIN32)
	@$(CC) -nologo $(shell echo $(OBJS) | $(TRANSFORM_TO_DOS_PATH) )	\
		-Fe$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH) )		\
		-link $(LDFLAGS) $(OS_LIBS) $(EXTRA_LIBS)
else
	@$(CCC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) 
endif
endif

################################################################################

$(LIBRARY): $(OBJS)
	@echo Building library $(LIBRARY)
	@$(MAKE_OBJDIR)
	@rm -f $@
ifeq ($(OS_NAME),WIN32)
####	@$(AR) $(shell echo $(OBJS) | sed 's|/cygdrive/[a-zA-Z]/|/|g' | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g') $(AR_EXTRA_ARGS)
####	we build a @file because the command line can overrun the win32 bash
####	command line limit (or something which crashes bash)....
	@echo -NOLOGO -OUT:"$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH))" > linkfile.1
	@echo $(OBJS) | $(TRANSFORM_TO_DOS_PATH) | sed 's|\\\\|\\|g'     > linkfile.2
	@echo $(AR_EXTRA_ARGS)									>linkfile.3
	@lib @linkfile.1 @linkfile.2 @linkfile.3
	@rm linkfile.[123456]
else
	@cd $(OBJDIR) && $(AR) $(SHORT_OBJS) $(AR_EXTRA_ARGS)
endif
	@$(RANLIB) $@

################################################################################

$(SHARED_LIBRARY): $(OBJS)
	@$(MAKE_OBJDIR)
	@rm -f $@
ifeq ($(ABI_FE), Win32)
ifeq ($(OS_NAME), MINGW32)
	@dllwrap --dllname=$(LIBDIR)/$(LIBRARY_NAME).dll \
	--implib=$(LIBDIR)/lib$(LIBRARY_NAME)dll.a \
	--driver-name=g++  \
	$(OBJS) $(EXTRA_LIBS) $(OS_LIBS)
else
	$(LINK_DLL) -MAP $(DLLBASE) $(OS_LIBS) \
	-implib:$(shell echo $(LIBRARY) | $(TRANSFORM_TO_DOS_PATH) )	\
	$(shell echo $(EXTRA_LIBS) | $(TRANSFORM_TO_DOS_PATH) ) \
	$(shell echo $(OBJS) | $(TRANSFORM_TO_DOS_PATH) )
endif
else
	$(MKSHLIB) -o $@ $(OBJS) $(EXTRA_LIBS) $(OS_LIBS)
endif

################################################################################

$(PLUGIN): $(OBJS)
	@$(MAKE_OBJDIR)
	@rm -f $@
ifeq ($(ABI_FE), Win32)
ifeq ($(OS_NAME), MINGW32)
	@dllwrap --dllname=$(PLUGINDIR)/$(LIBRARY_NAME).dll \
	--implib=$(PLUGINDIR)/lib$(LIBRARY_NAME)dll.a \
	--driver-name=g++  \
	$(OBJS) $(EXTRA_LIBS) $(OS_LIBS)
else
	@$(LINK_DLL) -MAP $(DLLBASE) $(OS_LIBS) \
	$(shell echo $(EXTRA_LIBS) | $(TRANSFORM_TO_DOS_PATH) ) \
	$(shell echo $(OBJS) | $(TRANSFORM_TO_DOS_PATH) )
endif
else
	$(MKSHLIB) -o $@ $(OBJS) $(EXTRA_LIBS) $(OS_LIBS)
endif

################################################################################

ifeq ($(OS_NAME), WIN32)
$(RCOBJS): $(RCSRCS)
	@$(MAKE_OBJDIR)
	@$(RC) /fo$(shell echo $(RCOBJS) | $(TRANSFORM_TO_DOS_PATH) )	\
		$(ABI_INCS) $(ABI_TMDEFS) $(RCSRCS)
	@echo $(RCOBJS) finished
endif

ifeq ($(OS_NAME), MINGW32)
$(RCOBJS): $(RCSRCS)
	@$(MAKE_OBJDIR)
	@$(RC) --verbose -i $(RCSRCS) $(ABI_TMDEFS) $(MINGRES_INCS) -o $(RCOBJS)
	@echo $(RCOBJS) finished
endif

ifeq ($(OS_NAME), MACOSX)
$(OBJDIR)/%.rsrc: %.r
	@$(MAKE_OBJDIR)
	@$(REZ) -o $@ $(ABI_MAC_REZ_OPTS) $<
	@echo $(REZOBJS) finished
endif

###############################################################################
## Rule for building .cpp sources in the current directory into .o's in $(OBJDIR)
###############################################################################

$(OBJDIR)/%.$(OBJ_SUFFIX): %.cpp
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME), WIN32)
	@$(CCC) -Fo$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH) ) -c $(CFLAGS) $<
else
	@echo $<:
	@$(CCC) -o $@ -c $(CFLAGS) $<
endif

###############################################################################
## Rule for building generated .cpp sources (in $(OBJDIR)) into .o's in $(OBJDIR)
###############################################################################

$(OBJDIR)/%.$(OBJ_SUFFIX): $(OBJDIR)/%.cpp
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME), WIN32)
	@$(CCC) -Fo$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH) ) -c	\
		$(CFLAGS) $(shell echo $< | $(TRANSFORM_TO_DOS_PATH) )
else
	@echo $<:
	@$(CCC) -o $@ -c $(CFLAGS) $<
endif

###############################################################################
## Rule for building .c sources in the current directory into .o's in $(OBJDIR)
###############################################################################

$(OBJDIR)/%.$(OBJ_SUFFIX): %.c
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME), WIN32)
	@$(CC) -Fo$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH) ) -c $(CFLAGS) $<
else
	@echo $<:
	@$(CC) -o $@ -c $(CFLAGS) $<
endif


###############################################################################
## Rule for building generated .c sources (in $(OBJDIR)) into .o's in $(OBJDIR)
###############################################################################

$(OBJDIR)/%.$(OBJ_SUFFIX): $(OBJDIR)/%.c
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME), WIN32)
	@$(CC) -Fo$(shell echo $@ | $(TRANSFORM_TO_DOS_PATH) ) -c	\
		$(CFLAGS) $(shell echo $< | $(TRANSFORM_TO_DOS_PATH) )
else
	@echo $<:
	@$(CC) -o $@ -c $(CFLAGS) $<
endif

################################################################################
# Special gmake rules.
################################################################################

#
# Re-define the list of default suffixes, so gmake won't have to churn through
# hundreds of built-in suffix rules for stuff we don't need.
#
.SUFFIXES:
.SUFFIXES: .a .$(OBJ_SUFFIX) .c .cpp .s .h .i .pl

#
# Fake targets.  Always run these rules, even if a file/directory with that
# name already exists.
#
.PHONY: all alltags clean export install libs realclean release abiclean

##################################################################
##################################################################

