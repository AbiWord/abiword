
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libpresentation.la

libpresentation_la_LIBADD = \
	xp/libxp.la

libpresentation_la_LDFLAGS = \
	$(PRESENTATION_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libpresentation_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

