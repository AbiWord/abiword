
noinst_LTLIBRARIES = libwin.la

AM_CPPFLAGS = \
	$(COLLAB_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../.. \
	-I$(srcdir)/../../.. \
	-I$(srcdir)/../../../core

libwin_la_SOURCES = \
	ap_Win32Dialog_GenericInput.cpp \
	ap_Win32Dialog_GenericInput.h \
	ap_Win32Dialog_GenericProgress.cpp \
	ap_Win32Dialog_GenericProgress.h \
	ServiceWin32AccountHandler.cpp \
	ServiceWin32AccountHandler.h

EXTRA_DIST = \
	ap_Win32Res_ServiceDialogs.rc \
	ap_Win32Res_DlgGenericInput.rc2 \
	ap_Win32Res_DlgGenericProgress.rc2
