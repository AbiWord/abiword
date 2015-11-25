
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(OTS_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\" \
	-DOTS_UI_DIR=\"$(ABIWORD_UIDIR)\"

libxp_la_SOURCES = \
	AbiOts.cpp

ui_gtk3 = ots.ui

uidir=$(ABIWORD_UIDIR)

ui_DATA = $(ui_gtk3)

EXTRA_DIST = \
	$(ui_gtk3)

