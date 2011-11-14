
noinst_LTLIBRARIES = libwin.la

AM_CPPFLAGS = \
	$(EPUB_CFLAGS) \
	-I$(srcdir)/../xp

libwin_la_SOURCES = \
    ap_Win32Dialog_EpubExportOptions.cpp \
    ap_Win32Dialog_EpubExportOptions.h

ap_Win32Res_EpubExportOptions.lo: ap_Win32Res_EpubExportOptions.rc $(libwin_la_OBJECTS)
	CPPFLAGS="$(COLLAB_RCFLAGS)" $(top_srcdir)/lt-compile-resource $< $@

EXTRA_DIST = \
	ap_Win32Res_EpubExportOptions.rc \
	ap_Win32Res_EpubExportOptions.rc2
