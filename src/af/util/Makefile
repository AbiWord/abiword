#! gmake

ABI_DEPTH=		..
include $(ABI_DEPTH)/config/abi_defs.mk

DIRS=			xp $(ABI_NATIVE)
LIBRARY_NAME=		AbiUtil
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

# note: we assume that this will be used as a static library
# note: only.  if we want to use it as a shared library, then
# note: we probably need to append $(SHARED_LIBRARY) to TARGETS
# note: and add a line to the intall section which copies it to
# note: the $(DIST)/bin directory on Windows.

OBJS=		$(ABI_NATIVE)/$(OBJDIR)/ut_debugmsg.$(OBJ_SUFFIX)	\
		xp/$(OBJDIR)/ut_hash.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_misc.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_pool.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_string.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_units.$(OBJ_SUFFIX)			\
		xp/$(OBJDIR)/ut_vector.$(OBJ_SUFFIX)

include $(ABI_DEPTH)/config/abi_rules.mk


libs:: $(TARGETS)

install:: libs
	$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	$(INSTALL) -m 444 $(TARGETS) $(DIST)/lib
