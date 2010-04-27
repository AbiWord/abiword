
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(COLLAB_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../.. \
	-I$(srcdir)/../../.. \
	-I$(srcdir)/../../packet \
	-I$(srcdir)/../../session

libxp_la_SOURCES = \
	lock.h \
	SynchronizedQueue.h \
	Synchronizer.cpp \
	Synchronizer.h
