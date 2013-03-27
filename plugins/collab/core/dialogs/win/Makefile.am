
noinst_LTLIBRARIES = libwin.la

AM_CPPFLAGS = \
	$(COLLAB_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../.. \
	-I$(srcdir)/../../.. \
	-I$(srcdir)/../../packet \
	-I$(srcdir)/../../session

libwin_la_SOURCES = \
	ap_Win32Dialog_CollaborationAccounts.cpp \
	ap_Win32Dialog_CollaborationAccounts.h \
	ap_Win32Dialog_CollaborationAddAccount.cpp \
	ap_Win32Dialog_CollaborationAddAccount.h \
	ap_Win32Dialog_CollaborationAddBuddy.cpp \
	ap_Win32Dialog_CollaborationAddBuddy.h \
	ap_Win32Dialog_CollaborationEditAccount.cpp \
	ap_Win32Dialog_CollaborationEditAccount.h \
	ap_Win32Dialog_CollaborationJoin.cpp \
	ap_Win32Dialog_CollaborationJoin.h \
	ap_Win32Dialog_CollaborationShare.cpp \
	ap_Win32Dialog_CollaborationShare.h

libwin_la_LIBADD = ap_Win32Res_CollaborationDialogs.lo

COLLAB_RCFLAGS = -I$(srcdir)
ap_Win32Res_CollaborationDialogs.lo:  $(srcdir)/ap_Win32Res_CollaborationDialogs.rc $(libwin_la_OBJECTS)
	CPPFLAGS="$(COLLAB_RCFLAGS)" $(top_srcdir)/lt-compile-resource $< $@

EXTRA_DIST = \
	ap_Win32Res_CollaborationDialogs.rc \
	ap_Win32Res_DlgCollaborationAccounts.rc2 \
	ap_Win32Res_DlgCollaborationAddAccount.rc2 \
	ap_Win32Res_DlgCollaborationAddBuddy.rc2 \
	ap_Win32Res_DlgCollaborationEditAccount.rc2 \
	ap_Win32Res_DlgCollaborationJoin.rc2 \
	ap_Win32Res_DlgCollaborationShare.rc2

