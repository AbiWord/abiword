
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(PRESENTATION_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	Presentation.cpp \
	Presentation.h

bindingsdir=$(ABIWORD_DATADIR)
bindings_DATA = \
	Presentation.xml

EXTRA_DIST = \
	$(bindings_DATA)
