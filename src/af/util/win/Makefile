#! gmake

ABI_DEPTH = ../..

include $(ABI_DEPTH)/config/abi_defs.mk

EXPORT_HEADERS=		ut_Win32Timer.h

HEADERS=		$(EXPORT_HEADERS)

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		ut_debugmsg.cpp		\
			ut_Win32Timer.cpp

INCLUDES=		-I$(DIST)/include -I. -I../xp

TARGETS=		$(OBJS)

include $(ABI_DEPTH)/config/abi_rules.mk

ifneq ($(strip $(EXPORT_HEADERS)),)
export:: $(EXPORT_HEADERS)
	@echo Exporting headers from `pwd`...
	@$(subst xxxx,$(DIST)/include,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(EXPORT_HEADERS) $(DIST)/include
endif

libs:: $(TARGETS)

