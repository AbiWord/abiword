
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = rsvg.la

rsvg_la_LIBADD = \
	xp/libxp.la \
	$(RSVG_LIBS) $(PNG_LIBS)

rsvg_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined

rsvg_la_SOURCES = 
nodist_EXTRA_rsvg_la_SOURCES = dummy.cpp

EXTRA_DIST = \
	plugin.m4

