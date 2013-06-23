
noinst_LTLIBRARIES = libunix.la

AM_CPPFLAGS = \
	$(COLLAB_CFLAGS) \
	-I$(srcdir)/../.. \
	-I$(srcdir)/../../.. \
	-I$(srcdir)/../../../core

libunix_la_SOURCES = \
	TelepathyChatroom.cpp \
	TelepathyChatroom.h \
	TelepathyBuddy.h \
	DTubeBuddy.h \
	TelepathyUnixAccountHandler.cpp \
	TelepathyUnixAccountHandler.h

servicefiledir = $(datadir)/dbus-1/services
servicefile_in_file = org.freedesktop.Telepathy.Client.AbiCollab.service.in
servicefile_DATA = $(servicefile_in_file:.service.in=.service)
%.service: %.service.in Makefile
	$(AM_V_GEN)sed -e "s|[@]bindir[@]|$(bindir)|" $< > $@

clientfiledir = $(datarootdir)/telepathy/clients
clientfile_DATA = AbiCollab.client

EXTRA_DIST= \
	$(servicefile_in_file) \
	$(clientfile_DATA)

CLEANFILES = \
	$(servicefile_DATA)
