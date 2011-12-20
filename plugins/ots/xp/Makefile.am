
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(OTS_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\" \
	-DOTS_UI_DIR=\"$(ABIWORD_UIDIR)\"

libxp_la_SOURCES = \
	AbiOts.cpp

uidir=$(ABIWORD_UIDIR)
if TOOLKIT_GTK2
ui_DATA = \
	ots-2.ui
else
ui_DATA = \
	ots.ui
endif

EXTRA_DIST = \
	$(ui_DATA)

