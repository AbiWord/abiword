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
## The Original Code is AbiSource Utilities.
## 
## The Initial Developer of the Original Code is AbiSource, Inc.
## Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
## All Rights Reserved. 
## 
## Contributor(s):
##  

##################################################################
##################################################################
## abi_rules.mk --  Makefile definitions for building AbiSource software.
## This is a makefile include.  It should be included before any
## other rules are defined.
##
## The general structure of an AbiSource Makefile should be:
##
##        #! gmake
##        ABI_DEPTH=<your depth in source tree from abi/src>
##        include $(ABI_DEPTH)/config/abi_defs.mk
##        <local declarations>
##        include $(ABI_DEPTH)/config/abi_rules.mk
##        <local rules>
##
##################################################################
##################################################################


ifdef LIBRARY_NAME
ifeq ($(OS_NAME), WINNT)
LIBRARY		= $(OBJDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION)_s.lib
SHARED_LIBRARY	= $(OBJDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION).dll
IMPORT_LIBRARY	= $(OBJDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION).lib

else

LIBRARY		= $(OBJDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION).$(LIB_SUFFIX)
SHARED_LIBRARY	= $(OBJDIR)/lib$(LIBRARY_NAME)$(LIBRARY_VERSION).$(DLL_SUFFIX)

endif
endif

ifndef TARGETS
ifeq ($(OS_NAME), WINNT)
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
		  $(addprefix $(OBJDIR)/,$(ASFILES:.s=.$(OBJ_SUFFIX)))
endif

ifeq ($(OS_NAME), WINNT)
OBJS += $(RES)
endif

ALL_TRASH		= $(TARGETS) $(OBJS) $(OBJDIR) LOGS TAGS $(GARBAGE) \
			  $(NOSUCHFILE) \
			  so_locations

ifdef DIRS
LOOP_OVER_DIRS		=					\
	@for d in $(DIRS); do					\
		if test -d $$d; then				\
			set -e;					\
			echo "cd $$d; $(MAKE) $@";		\
			$(MAKE) -C $$d $@;			\
			set +e;					\
		else						\
			echo "Skipping non-directory $$d...";	\
		fi;						\
	done
endif

################################################################################

all:: export libs install

export::
	+$(LOOP_OVER_DIRS)

libs::
	+$(LOOP_OVER_DIRS)

install::
	+$(LOOP_OVER_DIRS)

clean::
	rm -rf $(OBJS) so_locations $(NOSUCHFILE)
	+$(LOOP_OVER_DIRS)

clobber::
	rm -rf $(OBJS) $(TARGETS) $(OBJDIR) $(GARBAGE) so_locations $(NOSUCHFILE)
	+$(LOOP_OVER_DIRS)

realclean clobber_all::
	rm -rf $(wildcard *.OBJ *.OBJD) dist $(ALL_TRASH)
	+$(LOOP_OVER_DIRS)

$(PROGRAM): $(OBJS)
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME),WINNT)
	@$(CC) -nologo $(OBJS) -Fe$@ -link $(LDFLAGS) $(OS_LIBS) $(EXTRA_LIBS)
else
	@$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS)
endif

$(LIBRARY): $(OBJS)
	@$(MAKE_OBJDIR)
	@rm -f $@
	@$(AR) $(OBJS) $(AR_EXTRA_ARGS)
	@$(RANLIB) $@

$(SHARED_LIBRARY): $(OBJS)
	@$(MAKE_OBJDIR)
	@rm -f $@
ifeq ($(OS_NAME), WINNT)
	@$(LINK_DLL) -MAP $(DLLBASE) $(OS_LIBS) $(EXTRA_LIBS) $(OBJS)
else
	$(MKSHLIB) -o $@ $(OBJS) $(EXTRA_LIBS) $(OS_LIBS)
endif

ifeq ($(OS_NAME), WINNT)
$(RES): $(RESNAME)
	@$(MAKE_OBJDIR)
	@$(RC) -Fo$(RES) $(RESNAME)
	@echo $(RES) finished
endif

$(OBJDIR)/%.$(OBJ_SUFFIX): %.cpp
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME), WINNT)
	@$(CCC) -Fo$@ -c $(CFLAGS) $<
else
	@echo $<:
	@$(CCC) -o $@ -c $(CFLAGS) $<
endif

$(OBJDIR)/%.$(OBJ_SUFFIX): %.c
	@$(MAKE_OBJDIR)
ifeq ($(OS_NAME), WINNT)
	@$(CC) -Fo$@ -c $(CFLAGS) $*.c
else
	@$(CC) -o $@ -c $(CFLAGS) $*.c
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
.PHONY: all alltags clean export install libs realclean release

##################################################################
##################################################################

