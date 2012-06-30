
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(OTS_CFLAGS) \
	-DABI_BUILD_VERSION=\"$(VERSION)\" \
	-DOTS_UI_DIR=\"$(ABIWORD_UIDIR)\"

libxp_la_SOURCES = \
	AbiOts.cpp

ui_gtk2 = ots-2.ui

ui_gtk3 = ots.ui

uidir=$(ABIWORD_UIDIR)

if TOOLKIT_GTK2
ui_DATA = $(ui_gtk2)
else
ui_DATA = $(ui_gtk3)
endif

EXTRA_DIST = \
	$(ui_gtk2) $(ui_gtk3)

