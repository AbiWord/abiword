#! gmake

ABI_DEPTH = ../..

include $(ABI_DEPTH)/config/abi_defs.mk

EXPORT_HEADERS=		ut_assert.h		\
			ut_debugmsg.h		\
			ut_hash.h		\
			ut_misc.h		\
			ut_pool.h		\
			ut_stack.h		\
			ut_string.h		\
			ut_types.h		\
			ut_units.h		\
			ut_vector.h

HEADERS=		$(EXPORT_HEADERS)

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		ut_hash.cpp		\
			ut_misc.cpp		\
			ut_pool.cpp		\
			ut_stack.cpp		\
			ut_string.cpp		\
			ut_units.cpp		\
			ut_vector.cpp

INCLUDES=		-I$(DIST)/include -I.

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

export:: $(EXPORT_HEADERS)
	@echo Exporting headers from `pwd`...
	@$(subst xxxx,$(DIST)/include,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(EXPORT_HEADERS) $(DIST)/include

libs:: $(TARGETS)

