
noinst_LTLIBRARIES = libunix.la

AM_CPPFLAGS = \
	$(COLLAB_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../.. \
	-I$(srcdir)/../../.. \
	-I$(srcdir)/../../../core/packet \
	-I$(srcdir)/../../../core/session

libunix_la_SOURCES = \
	ap_UnixDialog_GenericInput.cpp \
	ap_UnixDialog_GenericInput.h \
	ap_UnixDialog_GenericProgress.cpp \
	ap_UnixDialog_GenericProgress.h \
	ServiceUnixAccountHandler.cpp \
	ServiceUnixAccountHandler.h

gladedir=$(ABIWORD_GLADEDIR)
glade_DATA = \
	ap_UnixDialog_GenericInput.glade \
	ap_UnixDialog_GenericProgress.glade

EXTRA_DIST = \
	$(glade_DATA)
