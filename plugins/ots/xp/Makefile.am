
noinst_LTLIBRARIES = libxp.la

libxp_la_CPPFLAGS = \
	$(OTS_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\" \
	-DOTS_GLADE_DIR=\"$(ABIWORD_GLADEDIR)\"

libxp_la_SOURCES = \
	AbiOts.cpp

gladedir=$(ABIWORD_GLADEDIR)
glade_DATA = \
	ots.glade

EXTRA_DIST = \
	$(glade_DATA)

