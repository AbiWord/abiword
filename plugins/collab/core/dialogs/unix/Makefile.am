
noinst_LTLIBRARIES = libunix.la

AM_CPPFLAGS = \
	$(COLLAB_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../.. \
	-I$(srcdir)/../../.. \
	-I$(srcdir)/../../packet \
	-I$(srcdir)/../../session

libunix_la_SOURCES = \
	ap_UnixDialog_CollaborationAccounts.cpp \
	ap_UnixDialog_CollaborationAccounts.h \
	ap_UnixDialog_CollaborationAddAccount.cpp \
	ap_UnixDialog_CollaborationAddAccount.h \
	ap_UnixDialog_CollaborationAddBuddy.cpp \
	ap_UnixDialog_CollaborationAddBuddy.h \
	ap_UnixDialog_CollaborationEditAccount.cpp \
	ap_UnixDialog_CollaborationEditAccount.h \
	ap_UnixDialog_CollaborationJoin.cpp \
	ap_UnixDialog_CollaborationJoin.h \
	ap_UnixDialog_CollaborationShare.cpp \
	ap_UnixDialog_CollaborationShare.h

uidir=$(ABIWORD_UIDIR)

ui_gtk2 = \
	ap_UnixDialog_CollaborationAccounts-2.ui \
	ap_UnixDialog_CollaborationAddAccount-2.ui \
	ap_UnixDialog_CollaborationAddBuddy-2.ui \
	ap_UnixDialog_CollaborationEditAccount-2.ui \
	ap_UnixDialog_CollaborationJoin-2.ui \
	ap_UnixDialog_CollaborationShare-2.ui

ui_gtk3 = \
	ap_UnixDialog_CollaborationAccounts.ui \
	ap_UnixDialog_CollaborationAddAccount.ui \
	ap_UnixDialog_CollaborationAddBuddy.ui \
	ap_UnixDialog_CollaborationEditAccount.ui \
	ap_UnixDialog_CollaborationJoin.ui \
	ap_UnixDialog_CollaborationShare.ui

if TOOLKIT_GTK2
ui_DATA = $(ui_gtk2)
else
ui_DATA = $(ui_gtk3)
endif

EXTRA_DIST = \
        $(ui_gtk2) $(ui_gtk3)

