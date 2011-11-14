noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(EPUB_CFLAGS) \
	-I$(srcdir)/../../imp/xp \
	-I$(srcdir)/../../exp/xp \
	-I$(srcdir)/../../dialogs/xp \
	-I$(srcdir)/../../dialogs/win \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	ie_impexp_EPUB.cpp\
	ie_impexp_EPUB.h
