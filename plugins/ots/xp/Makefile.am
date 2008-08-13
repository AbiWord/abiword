
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(OTS_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\" \
	-DOTS_UI_DIR=\"$(ABIWORD_UIDIR)\"

libxp_la_SOURCES = \
	AbiOts.cpp

uidir=$(ABIWORD_UIDIR)
ui_DATA = \
	ots.xml

EXTRA_DIST = \
	$(ui_DATA)

