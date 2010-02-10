if TOOLKIT_COCOA

else
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = garble.la

garble_la_LIBADD = \
	xp/libxp.la \
	$(GARBLE_LIBS) $(PNG_LIBS) -ljpeg

garble_la_LDFLAGS = \
	$(GARBLE_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

garble_la_SOURCES = 
nodist_EXTRA_garble_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

endif
