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


ifdef LIBRARY_NAME
ifeq ($(OS_NAME), WIN32)
LIBRARY		= $(LIBDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION)_s.lib
SHARED_LIBRARY	= $(LIBDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION).dll
IMPORT_LIBRARY	= $(LIBDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION).lib

else

LIBRARY		= $(LIBDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION).$(LIB_SUFFIX)
SHARED_LIBRARY	= $(LIBDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION).$(DLL_SUFFIX)

endif
endif

ifndef TARGETS
ifeq ($(OS_NAME), WIN32)
TARGETS		= $(LIBRARY) $(SHARED_LIBRARY) $(IMPORT_LIBRARY)
else
TARGETS		= $(LIBRARY) $(SHARED_LIBRARY)
endif
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

#
# Win32 resource file
#

ifeq ($(OS_NAME), WIN32)
RCOBJS		= $(addprefix $(OBJDIR)/,$(RCSRCS:.rc=.res))
OBJS		+= $(RCOBJS)
endif

#
# Trash which can be deleted
#

ALL_TRASH		= $(TARGETS) $(OBJS) $(OBJDIR) $(LIBDIR) LOGS TAGS $(GARBAGE) \
			  $(NOSUCHFILE) \
			  so_locations


ifdef DIRS
LOOP_OVER_DIRS		=						\
	@for d in $(DIRS); do						\
		if test -d $$d; then					\
			set -e;						\
			echo "$(MAKE) ABI_ROOT=$(ABI_ROOT) -C $$d $@";	\
			$(MAKE) ABI_ROOT=$(ABI_ROOT) -C $$d $@;		\
			set +e;						\
		else							\
			echo "Skipping non-directory $$d...";		\
		fi;							\
	done
endif

################################################################################

all:: build

build::
	@echo Building with [$(ABI_OPTIONS)].
	+$(LOOP_OVER_DIRS)

tidy::
	rm -rf $(OBJS)
	+$(LOOP_OVER_DIRS)

################################################################################

ifdef HELPER_PROGRAM
$(HELPER_PROGRAM): $(OBJS)
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME),WIN32)
	@$(CC) -nologo $(shell echo $(OBJS) | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')	\
		-Fe$(shell echo $@ | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')		\
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
####	@$(AR) $(shell echo $(OBJS) | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g') $(AR_EXTRA_ARGS)
####	we build a @file because the command line can overrun the win32 bash
####	command line limit (or something which crashes bash)....
	@echo -NOLOGO -OUT:"$(shell echo $@ | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')" 	>linkfile.1
	@echo $(OBJS) | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\|g' 				>linkfile.2
	@echo $(AR_EXTRA_ARGS)									>linkfile.3
	@lib @linkfile.1 @linkfile.2 @linkfile.3
	@rm linkfile.[123]
else
	@$(AR) $(OBJS) $(AR_EXTRA_ARGS)
endif
	@$(RANLIB) $@

################################################################################

$(SHARED_LIBRARY): $(OBJS)
	@$(MAKE_OBJDIR)
	@rm -f $@
ifeq ($(OS_NAME), WIN32)
	@$(LINK_DLL) -MAP $(DLLBASE) $(OS_LIBS) $(EXTRA_LIBS) $(subst /,\\,$(OBJS))
else
	$(MKSHLIB) -o $@ $(OBJS) $(EXTRA_LIBS) $(OS_LIBS)
endif

################################################################################

ifeq ($(OS_NAME), WIN32)
$(RCOBJS): $(RCSRCS)
	@$(MAKE_OBJDIR)
	@$(RC) /fo$(shell echo $(RCOBJS) | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')	\
		$(ABI_INCS) $(ABI_TMDEFS) $(RCSRCS)
	@echo $(RCOBJS) finished
endif

###############################################################################
## Rule for building .cpp sources in the current directory into .o's in $(OBJDIR)
###############################################################################

$(OBJDIR)/%.$(OBJ_SUFFIX): %.cpp
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME), WIN32)
	@$(CCC) -Fo$(shell echo $@ | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g') -c $(CFLAGS) $<
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
	@$(CCC) -Fo$(shell echo $@ | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g') -c	\
		$(CFLAGS) $(shell echo $< | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')
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
	@$(CC) -Fo$(shell echo $@ | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g') -c $(CFLAGS) $<
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
	@$(CC) -Fo$(shell echo $@ | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g') -c	\
		$(CFLAGS) $(shell echo $< | sed 's|//[a-zA-Z]/|/|g' | sed 's|/|\\\\|g')
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

