
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = wordperfect.la

wordperfect_la_LIBADD = \
	xp/libxp.la \
	$(WORDPERFECT_LIBS)

wordperfect_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

wordperfect_la_SOURCES = 
nodist_EXTRA_wordperfect_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

