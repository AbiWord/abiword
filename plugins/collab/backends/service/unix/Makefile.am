
noinst_LTLIBRARIES = libunix.la

AM_CPPFLAGS = \
	$(COLLAB_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../.. \
	-I$(srcdir)/../../.. \
	-I$(srcdir)/../../../core

libunix_la_SOURCES = \
	ap_UnixDialog_GenericInput.cpp \
	ap_UnixDialog_GenericInput.h \
	ap_UnixDialog_GenericProgress.cpp \
	ap_UnixDialog_GenericProgress.h \
	ServiceUnixAccountHandler.cpp \
	ServiceUnixAccountHandler.h

uidir=$(ABIWORD_UIDIR)

ui_gtk2 = \
	ap_UnixDialog_GenericInput-2.ui \
	ap_UnixDialog_GenericProgress-2.ui

ui_gtk3 = \
	ap_UnixDialog_GenericInput.ui \
	ap_UnixDialog_GenericProgress.ui

if TOOLKIT_GTK2
ui_DATA = $(ui_gtk2)
else
ui_DATA = $(ui_gtk3)
endif

EXTRA_DIST = \
	$(ui_gtk2) $(ui_gtk3)
